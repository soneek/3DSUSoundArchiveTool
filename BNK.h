typedef struct {
	bool exists;
	u16 type; // 0x6000 - single wav, 0x6001 - multiple wav
	int loop;
	u32 wavNumber;
	u32 offset;
	u8 startNote;
	u8 endNote;
	u32 baseNote;
	u8 volume;
	u8 pan;
	double realPan;
	u8 interpolation; // 0 = Polyphase (4-point), 1 = Linear, 2 = None
	u8 attack;
	u8 hold;
	u8 decay;
	s16 realDecay;
	u8 sustain;
	double sustainVol;
	u8 release;
	s16 realRelease;


	double getAttack() {
		return timeToTimecents(attackTable[attack] / 1000);
	}

	double getHold() {
		return timeToTimecents(holdTable[hold] / 1000);
	}

	double getVolume() {
		return 200 * abs(LogB(pow(((double)volume / 127.0000), 2), 10));
	}

	double getSustain() {
		if (sustain == 0)
			return 900;
		else
			return 200 * abs(LogB(pow(((double)sustain / 127.0000), 2), 10));
	}

	double getDecay() {
		sustainVol = 20 * LogB(pow(((double)sustain / 127.0000), 2), 10);
		//		return (sustainVol / decayTable[decay] / 1000 * 1.895183915);
		if (decay == 127)
			return -12000;
		else {
			if (sustain == 0)
				return timeToTimecents(-90.25 / decayTable[decay] / 1000);
			else
				return timeToTimecents(sustainVol / decayTable[decay] / 1000);
			//		return (sustainVol / decayTable[decay] / 1000 * 1.989943117);
		}
	}

	double getRelease() {
		sustainVol = 20 * LogB(pow(((double)sustain / 127.0000), 2), 10);
		if (release == 127)
			return -12000;
		else {
			if (sustain == 0)
				return timeToTimecents(-90.25 / decayTable[release] / 1000);
			else
				return timeToTimecents((-90.25 - sustainVol) / decayTable[release] / 1000);
		}
	}

	double getPan() {
		realPan = (pan - 64) * 7.936507937;
		if (realPan < 0)
			return realPan + 65536;
		else
			return realPan;
	}
} noteRegion;


typedef struct {
	u8 prgNumber; // General MIDI instrument number (0 - Acoustic Grand Piano, 127 - Gunshot)
	u32 offset;
	bool exists;
	u16 type; // 0x6000 - single wav; 0x6001 - multiple wav; 0x6002 - drum kit
	u32 noteCount;
	noteRegion notes[128];
} instrument;

typedef struct {
	bool exists;
	u32 warc;
	u32 number;
	u32 key;
} bankWav;

typedef struct {
	int id;
	u32 offset;
	u32 fileId;
	u32 strgId;
	u32 instTblOffset;
	u32 instCount;
	instrument instruments[128];
	u32 wavTblOffset;
	u32 wavCount;
	vector<bankWav> bankWavs;
	string name;
	string outfolder;
	void processBank(FILE * &csar){
		
		fseek(csar, files[fileId].fileOffset, SEEK_SET);
		printf("Processing bank %d at %X\n", id, ftell(csar));
		fseek(csar, 0x28, SEEK_CUR);
		tempHeader = ReadLE(csar, 16);
		fseek(csar, 2, SEEK_CUR);
		wavTblOffset = ReadLE(csar, 32);
		tempHeader = ReadLE(csar, 16);
		fseek(csar, 2, SEEK_CUR);
		instTblOffset = ReadLE(csar, 32);

		// Reading the wave table
		fseek(csar, files[fileId].fileOffset + 0x28 + wavTblOffset, SEEK_SET);
		wavCount = ReadLE(csar, 32);
		bankWavs.resize(wavCount);
		for (j = 0; j < (int)wavCount; j++) {
			bankWavs[j].exists = false;
			bankWavs[j].warc = ReadLE(csar, 32) - 0x5000000;
			typeHeader = ReadLE(csar, 32);
			if (typeHeader < 0xF000) {
				bankWavs[j].number = typeHeader;
				bankWavs[j].exists = true;
			}
			else
				continue;
		}

		// Reading instrument table
		fseek(csar, files[fileId].fileOffset + 0x28 + instTblOffset, SEEK_SET);
		instCount = ReadLE(csar, 32);
		//				instruments.resize((int)instCount);
		for (j = 0; j < (int)instCount; j++) {
			instruments[j].exists = false;
			fseek(csar, files[fileId].fileOffset + 0x28 + instTblOffset + 4 + j * 8, SEEK_SET);
			tempHeader = ReadLE(csar, 16);
			fseek(csar, 2, SEEK_CUR);
			if (tempHeader == 0x5900) {
				instruments[j].exists = true;
				instruments[j].offset = ReadLE(csar, 32);
				fseek(csar, files[fileId].fileOffset + 0x28 + instTblOffset + instruments[j].offset, SEEK_SET);

				//						printf("Reading info for instrument %d at %X\n", j, ftell(sar));

				instruments[j].type = ReadLE(csar, 16);
				fseek(csar, 6, SEEK_CUR);
				switch (instruments[j].type) {
				case 0x6000: // single wav

					instruments[j].noteCount = 1;


					instruments[j].notes[0].startNote = 0;
					instruments[j].notes[0].endNote = 127;
					break;
				case 0x6001: // multiple wav

					instruments[j].noteCount = ReadLE(csar, 32);
					for (k = 0; k < (int)instruments[j].noteCount; k++) {

						if (k > 0)
							instruments[j].notes[k].startNote = instruments[j].notes[k - 1].endNote + 1;
						else
							instruments[j].notes[k].startNote = 0;

						instruments[j].notes[k].endNote = ReadLE(csar, 8);
					}

					//							printf("Finished reading note ranges for instrument %d at %X\n", j, ftell(sar));
					fseek(csar, (ceil((double)instruments[j].noteCount / 4)) * 4 - (int)instruments[j].noteCount, SEEK_CUR);
					break;
				case 0x6002: // drum kit
					fseek(csar, 1, SEEK_CUR);
					instruments[j].noteCount = ReadLE(csar, 8) + 1;
					fseek(csar, 2, SEEK_CUR);
					break;
				}

				//						printf("%d wavs for instrument %d\n", instruments[j].noteCount, j);


				// Getting wav info offsets
				for (k = 0; k < (int)instruments[j].noteCount; k++) {
					instruments[j].notes[k].exists = false;
					//						printf("Reading info for note %d for instrument %d at %X\n", k, j, ftell(sar));

					tempHeader = ReadLE(csar, 16);
					fseek(csar, 2, SEEK_CUR);
					if (tempHeader == 0x5901) {
						instruments[j].notes[k].offset = ReadLE(csar, 32);
						instruments[j].notes[k].exists = true;

						//								printf("Note %d located at %X\n", k, files[fileId].fileOffset + 0x28 + instTblOffset + instruments[j].offset + 0x8 + instruments[j].notes[k].offset);


						if (instruments[j].type == 0x6002)
							instruments[j].notes[k].startNote = instruments[j].notes[k].endNote = k;	// For the drum kit, each wav is only used for one note				
					}
					else
						fseek(csar, 4, SEEK_CUR);
				}


				// Reading wav info

				for (k = 0; k < instruments[j].noteCount; k++) {
					if (instruments[j].notes[k].exists) {
						fseek(csar, files[fileId].fileOffset + 0x28 + instTblOffset + instruments[j].offset + 0x8 + instruments[j].notes[k].offset, SEEK_SET);
						//					printf("Checking note %d for instrument %d at %X\n", k, j, ftell(sar));

						instruments[j].notes[k].type = ReadLE(csar, 16);
						fseek(csar, 2, SEEK_CUR);
						if (instruments[j].notes[k].type == 0x6000) {
							//							printf("Reading note number %d at %X\n", k, ftell(sar));
							fseek(csar, 0xC, SEEK_CUR);
							instruments[j].notes[k].wavNumber = ReadLE(csar, 32);

							bankWavs[instruments[j].notes[k].wavNumber].exists = true;
							//							printf("Reading wave number %d\n", instruments[j].notes[k].wavNumber);
							fseek(csar, 4, SEEK_CUR);
							bankWavs[instruments[j].notes[k].wavNumber].key = instruments[j].notes[k].baseNote = ReadLE(csar, 32);
							instruments[j].notes[k].volume = ReadLE(csar, 32);
							instruments[j].notes[k].pan = ReadLE(csar, 32);
							fseek(csar, 6, SEEK_CUR);
							instruments[j].notes[k].interpolation = ReadLE(csar, 8);
							fseek(csar, 0xD, SEEK_CUR);
							instruments[j].notes[k].attack = ReadLE(csar, 8);
							instruments[j].notes[k].decay = ReadLE(csar, 8);
							instruments[j].notes[k].sustain = ReadLE(csar, 8);
							instruments[j].notes[k].hold = ReadLE(csar, 8);
							instruments[j].notes[k].release = ReadLE(csar, 8);
						}
						else if (instruments[j].notes[k].type == 0x6001) {
							fseek(csar, 0x1C, SEEK_CUR);
							instruments[j].notes[k].wavNumber = ReadLE(csar, 32);
							bankWavs[instruments[j].notes[k].wavNumber].exists = true;

							//									bankWavs[instruments[j].notes[k].wavNumber].exists = true;

							//							printf("Reading wave number %d\n", instruments[j].notes[k].wavNumber);
							fseek(csar, 4, SEEK_CUR);
							bankWavs[instruments[j].notes[k].wavNumber].key = instruments[j].notes[k].baseNote = ReadLE(csar, 32);
							instruments[j].notes[k].volume = ReadLE(csar, 32);
							instruments[j].notes[k].pan = ReadLE(csar, 32);
							fseek(csar, 6, SEEK_CUR);
							instruments[j].notes[k].interpolation = ReadLE(csar, 8);
							fseek(csar, 0xD, SEEK_CUR);
							instruments[j].notes[k].attack = ReadLE(csar, 8);
							instruments[j].notes[k].decay = ReadLE(csar, 8);
							instruments[j].notes[k].sustain = ReadLE(csar, 8);
							instruments[j].notes[k].hold = ReadLE(csar, 8);
							instruments[j].notes[k].release = ReadLE(csar, 8);
						}
						else
							continue;
					}
					else
						continue;
				}
			}
			else
				continue;
		}

		ofstream bankTemplate(outfolder + name + ".txt");
		stringstream bankTemplateText;
		string bankText;
		bankTemplateText << "[Samples]\n";


		for (k = 0; k < (int)wavCount; k++) {

			if (bankWavs[k].exists) {

				//						printf("Writing sample %X\n", bankWavs[k].number);
				bankTemplateText << "\n    SampleName=" << warcs[bankWavs[k].warc].name << "_" << bankWavs[k].number << "\n        SampleRate=" << to_string(warcs[bankWavs[k].warc].bWavs[bankWavs[k].number].sampleRate) << "\n        Key=" << to_string(bankWavs[k].key) << "\n        FineTune=0\n        Type=1\n";
				//								printf("sample %d", bankWavs[k].number);
			}
			else
				continue;
		}

		bankTemplateText << "\n\n[Instruments]\n";

		for (j = 0; j < (int)instCount; j++) {
			if (instruments[j].exists) {

				//						printf("Writing instrument %X\n", j);
				bankTemplateText << "\n    InstrumentName=Instrument" << j << "\n";
				for (k = 0; k < instruments[j].noteCount; k++) {
					if (instruments[j].notes[k].exists) {
						bankTemplateText << "\n        Sample=" << warcs[bankWavs[instruments[j].notes[k].wavNumber].warc].name << "_" << to_string(bankWavs[instruments[j].notes[k].wavNumber].number) << "\n";
						bankTemplateText << "            Z_LowKey=" << to_string(instruments[j].notes[k].startNote) << "\n";
						bankTemplateText << "            Z_HighKey=" << to_string(instruments[j].notes[k].endNote) << "\n";
						bankTemplateText << "            Z_LowVelocity=0\n";
						bankTemplateText << "            Z_HighVelocity=127\n";
						bankTemplateText << "            Z_overridingRootKey=" << to_string(instruments[j].notes[k].baseNote) << "\n";
						bankTemplateText << "            Z_initialAttenuation=" << to_string((int)floor(instruments[j].notes[k].getVolume())) << "\n";
						bankTemplateText << "            Z_pan=" << to_string((int)floor(instruments[j].notes[k].getPan())) << "\n";
						bankTemplateText << "            Z_attackVolEnv=" << to_string((int)instruments[j].notes[k].getAttack()) << "\n";
						bankTemplateText << "            Z_holdVolEnv=" << to_string((int)instruments[j].notes[k].getHold()) << "\n";
						bankTemplateText << "            Z_decayVolEnv=" << to_string((int)floor(instruments[j].notes[k].getDecay())) << "\n";
						bankTemplateText << "            Z_releaseVolEnv=" << to_string((int)floor(instruments[j].notes[k].getRelease())) << "\n";
						bankTemplateText << "            Z_sustainVolEnv=" << to_string((int)floor(instruments[j].notes[k].getSustain())) << "\n";
						bankTemplateText << "            Z_sampleModes=" << (int)warcs[bankWavs[instruments[j].notes[k].wavNumber].warc].bWavs[bankWavs[instruments[j].notes[k].wavNumber].number].loopFlag << "\n";
					}

					else
						continue;
				}

				//									bankTemplateText << "\n        GlobalZone\n            GZ_freqModLFO=64686\n            GZ_freqVibLFO=64686\n            GZ_Modulator=(MIDI1,NormalDirection,Unipolar,Linear), vibLfoToPitch, 0, (NoController,NormalDirection,Unipolar,Linear), 0\n\n            GZ_Modulator=(MIDI10,NormalDirection,Bipolar,Linear), pan, 1000, (NoController,NormalDirection,Unipolar,Linear), 0\n\n            GZ_Modulator=(ChannelPressure,NormalDirection,Unipolar,Linear), vibLfoToPitch, 0, (NoController,NormalDirection,Unipolar,Linear), 0\n\n            GZ_Modulator=(MIDI91,NormalDirection,Unipolar,Linear), reverbEffectsSend, 1000, (NoController,NormalDirection,Unipolar,Linear), 0\n\n            GZ_Modulator=(MIDI93,NormalDirection,Unipolar,Linear), chorusEffectsSend, 1000, (NoController,NormalDirection,Unipolar,Linear), 0\n\n";

			}
		}
		bankTemplateText << "\n\n[Presets]\n";
		for (j = 0; j < (int)instCount; j++) {
			if (instruments[j].exists) {

				//						printf("Writing preset %d", j);
				//	if (instruments[j].type == 0x6002 || j == 127) { // drum kit 
				bankTemplateText << "\n    PresetName=Program" << j << "Drum\n        Bank=128\n        Program=" << j << "\n";
				bankTemplateText << "\n        Instrument=Instrument" << j << "\n            L_LowKey=0\n            L_HighKey=127\n            L_LowVelocity=0\n            L_HighVelocity=127\n\n";
				//	}
				bankTemplateText << "\n    PresetName=Program" << j << "\n        Bank=0\n        Program=" << j << "\n";
				bankTemplateText << "\n        Instrument=Instrument" << j << "\n            L_LowKey=0\n            L_HighKey=127\n            L_LowVelocity=0\n            L_HighVelocity=127\n\n";
			}
			else
				continue;
		}

		bankTemplateText << "\n[Info]\nVersion=2.1\nEngine=EMU8000 \nName=" << name << "\nROMName=\nROMVersion=0.0\nDate=\nDesigner=\nProduct=\nCopyright=\nEditor=Awave Studio v10.6  \nComments=\n";
		bankText = bankTemplateText.str();
		//							regex e("(-2147483648)");
		//							regex_replace(bankText, e, "-12000");
		bankTemplate << bankText;
		bankTemplate.close();
	}
} bank;

vector<bank> banks;