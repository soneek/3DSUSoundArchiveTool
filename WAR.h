typedef struct {
	int id;
	u32 offset;
	u32 fileId;
	u16 sectionCount;
	u32 infoOffset;
	u32 infoLength;
	u32 fileOffset;
	u32 fileLength;
	u32 wavCount;
	vector<bwav> bWavs;
	string name;
	string warcfolder;
	void processWarc(FILE * &csar) {
		
		fseek(csar, files[fileId].fileOffset, SEEK_SET);
		printf("Processing wave archive %d at %X\n", id, ftell(csar));
		fseek(csar, 0x10, SEEK_CUR); // Skipping to pointers for INFO and FILE sections of wave archive
		sectionCount = ReadLE(csar, 16);
		fseek(csar, 2, SEEK_CUR);
		for (j = 0; j < (int)sectionCount; j++) {
			tempHeader = ReadLE(csar, 16);
			fseek(csar, 2, SEEK_CUR);
			if (tempHeader == 0x6800) {
				// Found INFO pointer
				infoOffset = ReadLE(csar, 32);
				infoLength = ReadLE(csar, 32);
				//						printf("Found INFO pointer\n");
			}
			else if (tempHeader == 0x6801) {
				// Found FILE pointer
				fileOffset = ReadLE(csar, 32);
				fileLength = ReadLE(csar, 32);
				//						printf("Found FILE pointer\n");
			}
		}
		// Going to INFO table
		fseek(csar, files[fileId].fileOffset + infoOffset + 8, SEEK_SET);
		wavCount = ReadLE(csar, 32);
		//				printf("%d wavs in this archive\n", wavCount);
		bWavs.resize((int)wavCount);
		// Fetching b(c/f)wav offsets...
		for (j = 0; j < int(wavCount); j++) {
			fseek(csar, files[fileId].fileOffset + infoOffset + 12 + j * 12, SEEK_SET);
			tempHeader = ReadLE(csar, 16);
			fseek(csar, 2, SEEK_CUR);
			if (tempHeader == 0x1f00) {
				bWavs[j].offset = ReadLE(csar, 32); // Found string pointer
				bWavs[j].size = ReadLE(csar, 32);
				// Now to get b(c/f)wav info
				fseek(csar, files[fileId].fileOffset + fileOffset + 8 + bWavs[j].offset, SEEK_SET);
				typeHeader = ReadBE(csar, 32);
				fseek(csar, 0xC, SEEK_CUR);
				//						printf("Wav %d out of %d - %X\n", j, wavCount, typeHeader);
				if (typeHeader != 0x43574156)
					continue;
				else {
					bWavs[j].sectionCount = ReadLE(csar, 16);
					fseek(csar, 2, SEEK_CUR);
					for (k = 0; k < (int)bWavs[j].sectionCount; k++) {
						tempHeader = ReadLE(csar, 16);
						fseek(csar, 2, SEEK_CUR);
						if (tempHeader == 0x7000) {
							// Found INFO pointer
							bWavs[j].infoOffset = ReadLE(csar, 32);
							bWavs[j].infoLength = ReadLE(csar, 32);
						}
						else if (tempHeader == 0x7001) {
							// Found FILE pointer
							bWavs[j].dataOffset = ReadLE(csar, 32);
							bWavs[j].dataLength = ReadLE(csar, 32);
						}
					}
					fseek(csar, files[fileId].fileOffset + fileOffset + 8 + bWavs[j].offset + bWavs[j].infoOffset + 8, SEEK_SET);
					bWavs[j].type = ReadLE(csar, 8);
					bWavs[j].loopFlag = ReadLE(csar, 8);
					fseek(csar, 2, SEEK_CUR);
					bWavs[j].sampleRate = ReadLE(csar, 32);
					//							printf("sample rate = %d\n", bWavs[j].sampleRate);
					bWavs[j].loopStart = ReadLE(csar, 32);
					bWavs[j].loopEnd = ReadLE(csar, 32);
					fseek(csar, 4, SEEK_CUR);
					bWavs[j].channels = ReadLE(csar, 32);
				}
			}
			else {
				fseek(csar, 8, SEEK_CUR);
			}
		}



#if defined(_WIN32)
		_mkdir(warcfolder.c_str());
#else 
		mkdir(warcfolder.c_str(), 0777); // notice that 777 is different than 0777
#endif
		files[fileId].fileName = name + "." + files[fileId].type;
		/*
		fseek(csar, files[fileId].fileOffset, SEEK_SET);
		ofstream tempFile(warcfolder + files[fileId].fileName, ofstream::binary);
		buffer = (char*) malloc((int)files[fileId].size + 1);
		fread(buffer, 1, (int)files[fileId].size, sar);
		tempFile.write(buffer, (int)files[fileId].size);
		tempFile.close();
		*/
		// Exporting b(c/f)wavs
		for (j = 0; j < wavCount; j++) {
		fseek(csar, files[fileId].fileOffset + fileOffset + 8 + bWavs[j].offset, SEEK_SET);

		ofstream tempFile(warcfolder + name + "_" + to_string(j) + ".bcwav", ofstream::binary);

		buffer = (char*) malloc((int)bWavs[j].size + 1);
		fread(buffer, 1, (int)bWavs[j].size, csar);
		tempFile.write(buffer, (int)bWavs[j].size);
		tempFile.close();

		}

		
	}
} warc;

vector<warc> warcs;