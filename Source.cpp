#include <iostream>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <regex>
#include <vector>
#include <direct.h>
#include <math.h>
#include <sar.h>
#include <STRGSection.h>
#include <WAV.h>
#include <WAR.h>
#include <BNK.h>
#include <GRP.h>
#include <INFOSection.h>

using namespace std;

int main(int argc, const char* argv[])
{
	FILE * sar , tempBankTemplate;
	u32 id;
	
	u32 (*Read)(FILE *f, u32 b);

	
	stringstream tempString;
	string basefolder;
//	vector<strg> *names;
	
	if (argc != 2) {
		printf("Usage:\n%s archive.bcsar", argv[0]);
		exit(0);
	}	
	else
		sar = fopen(argv[1], "rb");
	typeHeader = ReadBE(sar, 32);
	if ((typeHeader != 0x43534152) && (typeHeader != 0x46534152)) {
		printf("Not a CSAR or FSAR archive");
		goto end;
	}
	else {
		printf("CSAR archive opened\n");
		csarHeader cHead;
		strgSection strgSec;
		infoSection infoSec;
		fileSection fileSec;
		cHead.id = typeHeader;
		cHead.order = ReadBE(sar, 16);
		if (cHead.order == 0xFFFE) {
//			u32 (*Read)(FILE *f, u32 b);
			Read = ReadLE;
			printf("little endian\n");
		}
		else if (cHead.order == 0xFEFF) {
			u32 (*Read)(FILE *f, s32 b);
			Read = ReadBE;
			printf("big endian\n");
		}
		else {
			printf("Not a valid sound archive");
			goto end;
		}
		basefolder = string(argv[1]) + "_ext";
//		basefolder.erase(-6);

		#if defined(_WIN32)
						_mkdir(basefolder.c_str());
			#else 
						mkdir(basefolder.c_str(), 0777); // notice that 777 is different than 0777
			#endif

		cHead.headerSize = Read(sar, 16);
		fseek(sar, 4, SEEK_CUR);
		cHead.fileSize = Read(sar, 32);
		cHead.sectionCount = Read(sar, 16);
		fseek(sar, 2, SEEK_CUR);
		for (i = 0; i < (int)cHead.sectionCount; i++) {
//			printf("%X\n", ftell(sar));
			tempHeader = Read(sar, 16);
			fseek(sar, 2, SEEK_CUR);
			if (tempHeader == 0x2000) {
				// Found STRG pointer
				strgSec.offset = Read(sar, 32);
				strgSec.size = Read(sar, 32);
				printf("STRG section located at %X\n", strgSec.offset);
			}
			else if (tempHeader == 0x2001) {
				// Found INFO pointer
				infoSec.offset = Read(sar, 32);
				infoSec.size = Read(sar, 32);
				printf("INFO section located at %X\n", infoSec.offset);
			}
			else if (tempHeader == 0x2002) {
				// Found FILE pointer
				fileSec.offset = Read(sar, 32);
				fileSec.size = Read(sar, 32);
				printf("FILE section located at %X\n", fileSec.offset);
			}
			else {
				printf("Unrecognized section pointer\n");
				goto end;
			}
		}
		
		// Reading STRG section

		strgSec.getStrings(sar);
		

		// Reading INFO Section

		infoSec.processInfoTable(sar);

		// Reading the file table
		
		infoSec.readFileTable(sar);

		// Reading file info
		for (i = 0; i < (int)infoSec.fileCount; i++) {
			files[i].internalFile = false;
			if (i < (int)infoSec.fileCount - 1)
				files[i].fileInfoLength = files[i + 1].offset - files[i].offset - 0xC;
			else
				files[i].fileInfoLength = infoSec.fileTableEnd - infoSec.fileTableOffset - files[i].offset - 0xC;
			fseek(sar, infoSec.offset + 8 + infoSec.fileTableOffset + files[i].offset, SEEK_SET);
			tempHeader = Read(sar, 16);
			if (tempHeader == 0x220D) {
				fseek(sar, infoSec.offset + 8 + infoSec.fileTableOffset + files[i].offset + 0xC, SEEK_SET);
				for (j = 0; j < int(files[i].fileInfoLength) - 1; j++) {
					tempChar = Read(sar, 8);
					if (tempChar != 0x00) {
						files[i].fileLocation[j] = tempChar;
//						cout << files[i].fileLocation[j];
					}
				}
//				printf("\n");
			}
			else if (tempHeader == 0x220C) {
				files[i].internalFile = true;
				fseek(sar, infoSec.offset + 8 + infoSec.fileTableOffset + files[i].offset + 0xC, SEEK_SET);
				tempHeader = Read(sar, 16);
				fseek(sar, 2, SEEK_CUR);
				if (tempHeader != 0xFFFF) {
					tempOffset = Read(sar, 32);
					if (tempOffset == 0xFFFFFFFF)
						continue;
					files[i].fileOffset = tempOffset + fileSec.offset + 8;
					files[i].size = Read(sar, 32);
					fseek(sar, files[i].fileOffset, SEEK_SET);
					typeHeader = ReadBE(sar, 32);
					switch(typeHeader) {
					case 0x43475250: // CGRP
						files[i].type.assign("bcgrp");
						break;
					case 0x46475250: // FGRP
						files[i].type.assign("bfgrp");
						break;
					case 0x43424E4B: // CBNK
						files[i].type.assign("bcbnk");
						break;
					case 0x46424E4B: // FBNK
						files[i].type.assign("bfbnk");
						break;
					case 0x43534551: // CSEQ
						files[i].type.assign("bcseq");
						break;
					case 0x46534551: // FSEQ
						files[i].type.assign("bfseq");
						break;
					case 0x43574152: // CWAR
						files[i].type.assign("bcwar");
						break;
					case 0x46574152: // FWAR
						files[i].type.assign("bfwar");
						break;
					case 0x43575344:  // CWSD
						files[i].type.assign("bcwsd");
						break;
					case 0x46575344: // FWSD
						files[i].type.assign("bfwsd");
						break;
					default:
						break;
					}
//					printf("File %d is a %s, located at %X\n", i, files[i].type, files[i].fileOffset + fileSec.offset + 8);
				}
				else {
					printf("Unrecognized file pointer for file %d at offset %X\n", i, ftell(sar) - 4);
					goto end;
				}
			}
			else {
				continue;
			}
		}
		
		// Reading the wave archive table
		fseek(sar, infoSec.offset + 8 + infoSec.warcTableOffset, SEEK_SET);
		infoSec.warcCount = Read(sar, 32);
		warcs.resize(infoSec.warcCount);
		for (i = 0; i < (int)infoSec.warcCount; i++) {
			warcs[i].id = i;
			tempHeader = Read(sar, 16);
			fseek(sar, 2, SEEK_CUR);
			if (tempHeader == 0x2207) 
				warcs[i].offset = Read(sar, 32);
			else {
				printf("Unrecognized wave archive pointer\n");
				goto end;
			}	
		}

		string outfolder = basefolder + "/WaveArchives";
			#if defined(_WIN32)
						_mkdir(outfolder.c_str());
			#else 
						mkdir(outfolder.c_str(), 0777); // notice that 777 is different than 0777
			#endif
		// Reading wave archive info {WARCING}
		for (i = 0; i < infoSec.warcCount; i++) {
			
			fseek(sar, infoSec.offset + 8 + infoSec.warcTableOffset + warcs[i].offset, SEEK_SET);
			if (i < (int)infoSec.warcCount - 1)
				warcs[i].infoLength = warcs[i + 1].offset - warcs[i].offset;
			else
				warcs[i].infoLength = infoSec.groupTableOffset - infoSec.warcTableOffset - warcs[i].offset;
			warcs[i].fileId = Read(sar,32);
			files[warcs[i].fileId].id = i;
			files[warcs[i].fileId].type.assign("bcwar");
			fseek(sar, 8, SEEK_CUR);
			if (warcs[i].infoLength > 0xC) 
				warcs[i].name = string(strgSec.names[Read(sar, 32)].name);
			else {
				tempString.str("");
				tempString << "WARC_" << hex << i;
				warcs[i].name = tempString.str();
			}

			warcs[i].warcfolder = outfolder + "/" + warcs[i].name + "/";
			if (files[warcs[i].fileId].internalFile) {

				warcs[i].processWarc(sar);


			}
			else
				continue;
//			cout << "Wave Archive " << hex << i << " - " << warcs[i].name << ".bcwar" << endl; 
		}

		// Reading the bank table
		infoSec.readBankTable(sar);

		string bankFolder = basefolder + "/Banks/";
		#if defined(_WIN32)
					_mkdir(bankFolder.c_str());
		#else 
					mkdir(bankFolder.c_str(), 0777); // notice that 777 is different than 0777
		#endif 

		// Reading bank info {BANKING}

					for (i = 0; i < infoSec.bankCount; i++) {
						
						fseek(sar, infoSec.offset + 8 + infoSec.bankTableOffset + banks[i].offset, SEEK_SET);
						printf("Checking bank %d at %X\n", i, ftell(sar));
						banks[i].fileId = Read(sar, 32);
						files[banks[i].fileId].id = i;
						files[banks[i].fileId].type.assign("bcbnk");
						fseek(sar, 0xC, SEEK_CUR);
						
						banks[i].strgId = Read(sar, 32);
			//			printf("Bank %d has string ID %X", i, banks[i].strgId);
						banks[i].name = string(strgSec.names[banks[i].strgId].name);
		//				printf("Checking bank %s at %X", banks[i].name, ftell(sar));
						files[banks[i].fileId].fileName = banks[i].name + "." + files[banks[i].fileId].type;

			//			cout << banks[i].name << endl;

						banks[i].outfolder = bankFolder + banks[i].name + "/";
#if defined(_WIN32)
						_mkdir(banks[i].outfolder.c_str());
#else 
						mkdir(banks[i].outfolder.c_str(), 0777); // notice that 777 is different than 0777
#endif 
						if (files[banks[i].fileId].internalFile) {
					
							
							banks[i].processBank(sar);

							

							//				printf("Finished checking bank %d\n", i);




							

							/*				Wrting bank to file
											ofstream tempFile(outfolder + "/" + files[banks[i].fileId].fileName, ofstream::binary);
											buffer = (char*)malloc((int)files[banks[i].fileId].size + 1);
											fread(buffer, 1, (int)files[banks[i].fileId].size, sar);
											tempFile.write(buffer, (int)files[banks[i].fileId].size);
											tempFile.close(); */


							cout << "Bank " << hex << i << " - " << banks[i].name << ".bcbnk" << endl;

						}
						else
							continue;
					}

							// Reading the audio table
							fseek(sar, infoSec.offset + 8 + infoSec.audioTableOffset, SEEK_SET);
							infoSec.audioCount = Read(sar, 32);
							sounds.resize(infoSec.audioCount);
							for (i = 0; i < (int)infoSec.audioCount; i++) {
								tempHeader = Read(sar, 16);
								fseek(sar, 2, SEEK_CUR);
								if (tempHeader == 0x2200)
									sounds[i].offset = Read(sar, 32);
								else {
									printf("Unrecognized audio pointer\n");
									goto end;
								}
							}

							// Reading audio info
							for (i = 0; i < (int)infoSec.audioCount; i++) {
								if (sounds[i].offset > 0xffff)
									continue;
								else {
									fseek(sar, infoSec.offset + 8 + infoSec.audioTableOffset + sounds[i].offset, SEEK_SET);
									sounds[i].fileId = Read(sar, 32);
									fseek(sar, 8, SEEK_CUR);
									tempHeader = Read(sar, 16);
									fseek(sar, 10, SEEK_CUR);
									sounds[i].strgId = Read(sar, 32);
									sounds[i].name = string(strgSec.names[sounds[i].strgId].name);
									files[sounds[i].fileId].fileName = sounds[i].name + "." + files[sounds[i].fileId].type;
									switch (tempHeader) {
									case 0x2201: // External stream
										break;
									case 0x2202: // WSD
										break;
									case 0x2203: // Sequence
										fseek(sar, 0x44, SEEK_CUR);
										sounds[i].bank = Read(sar, 16);
										files[sounds[i].fileId].bank = sounds[i].bank;
										fseek(sar, files[sounds[i].fileId].fileOffset, SEEK_SET);
										ofstream tempFile(banks[sounds[i].bank].outfolder + files[sounds[i].fileId].fileName, ofstream::binary);
										buffer = (char*)malloc((int)files[sounds[i].fileId].size + 1);
										fread(buffer, 1, (int)files[sounds[i].fileId].size, sar);
										tempFile.write(buffer, (int)files[sounds[i].fileId].size);
										tempFile.close();
										cout << sounds[i].name << " uses bank: " << banks[sounds[i].bank].name << endl;
										break;
									}
								}
							}

							// Reading the player table
							fseek(sar, infoSec.offset + 8 + infoSec.playerTableOffset, SEEK_SET);
							infoSec.playerCount = Read(sar, 32);
							players.resize(infoSec.playerCount);
							for (i = 0; i < (int)infoSec.playerCount; i++) {
								tempHeader = Read(sar, 16);
								fseek(sar, 2, SEEK_CUR);
								if (tempHeader == 0x2209)
									players[i].offset = Read(sar, 32); // Found end of file table
								else {
									printf("Unrecognized player pointer\n");
									goto end;
								}
							}

							// Reading the sequence/wave sound set table
							fseek(sar, infoSec.offset + 8 + infoSec.setTableOffset, SEEK_SET);
							infoSec.setCount = Read(sar, 32);
							sets.resize(infoSec.setCount);
							for (i = 0; i < (int)infoSec.setCount; i++) {
								tempHeader = Read(sar, 16);
								fseek(sar, 2, SEEK_CUR);
								if (tempHeader == 0x2204)
									sets[i].offset = Read(sar, 32);
								else {
									printf("Unrecognized sequence/wave sound set pointer\n");
									goto end;
								}
							}

							// Reading sequence/wave sound set info
							for (i = 0; i < (int)infoSec.setCount; i++) {
								if (sets[i].offset == 0xffffffff)
									continue;
								else {
									fseek(sar, infoSec.offset + 8 + infoSec.setTableOffset + sets[i].offset, SEEK_SET);
									//				printf("Sound Set %d info located at %X\n", i, ftell(sar));
									typeHeader = Read(sar, 32);
									if (typeHeader == 0xffffffff)
										continue;
									else {
										fseek(sar, 0x18, SEEK_CUR);
										sets[i].strgId = Read(sar, 32);
										sets[i].name = string(strgSec.names[sets[i].strgId].name);
										fseek(sar, 4, SEEK_CUR);
										sets[i].fileId = Read(sar, 32);
										files[sets[i].fileId].fileName = sets[i].name + "." + files[sets[i].fileId].type + "set";
										// Wrting set
										/*					if (files[sets[i].fileId].bank != NULL) {
																fseek(sar, files[sets[i].fileId].fileOffset, SEEK_SET);
																ofstream tempFile(banks[files[sets[i].fileId].bank].outfolder + files[sets[i].fileId].fileName, ofstream::binary);
																buffer = (char*) malloc((int)files[sets[i].fileId].size + 1);
																fread(buffer, 1, (int)files[sets[i].fileId].size, sar);
																tempFile.write(buffer, (int)files[sets[i].fileId].size);
																tempFile.close();
																} */
									}
								}
							}

							string groupFolder = basefolder + "/Groups";
#if defined(_WIN32)
							_mkdir(groupFolder.c_str());
#else 
							mkdir(groupFolder.c_str(), 0777); // notice that 777 is different than 0777
#endif
							// Reading the group table
							fseek(sar, infoSec.offset + 8 + infoSec.groupTableOffset, SEEK_SET);
							infoSec.groupCount = Read(sar, 32);
							groups.resize(infoSec.groupCount);
							for (i = 0; i < (int)infoSec.groupCount; i++) {
								tempHeader = Read(sar, 16);
								fseek(sar, 2, SEEK_CUR);
								if (tempHeader == 0x2208)
									groups[i].offset = Read(sar, 32);
								else {
									printf("Unrecognized group pointer\n");
									goto end;
								}
							}

							// Read group info
							for (i = 0; i < (int)infoSec.groupCount; i++) {
								fseek(sar, infoSec.offset + 8 + infoSec.groupTableOffset + groups[i].offset, SEEK_SET);
								groups[i].fileId = Read(sar, 32);
								fseek(sar, 4, SEEK_CUR);
								groups[i].name = string(strgSec.names[Read(sar, 32)].name);
								files[groups[i].fileId].fileName = groups[i].name + "." + files[groups[i].fileId].type;
								groups[i].outfolder = groupFolder + "/" + groups[i].name + "/";
#if defined(_WIN32)
								_mkdir(groups[i].outfolder.c_str());
#else 
								mkdir(groups[i].outfolder.c_str(), 0777); // notice that 777 is different than 0777
#endif 

								//			Wrting group to file
								fseek(sar, files[groups[i].fileId].fileOffset, SEEK_SET);
								groups[i].processGroup(sar);

								ofstream tempFile(groups[i].outfolder + files[groups[i].fileId].fileName, ofstream::binary);
								buffer = (char*)malloc((int)files[groups[i].fileId].size + 1);
								fread(buffer, 1, (int)files[groups[i].fileId].size, sar);
								tempFile.write(buffer, (int)files[groups[i].fileId].size);
								tempFile.close();

								//			cout << "Group " << hex << i << " - " << groups[i].name << ".bcgrp" << endl;
							}

							for (i = 0; i < (int)infoSec.fileCount; i++) {
								if (files[i].internalFile && files[i].type == "pkm") {
									fseek(sar, files[i].fileOffset, SEEK_SET);
									/*				switch (files[i].type) {
													case "bcseq":
													ofstream tempFile("Sequence/" + files[i].fileName, ofstream::binary);
													break;
													case "bcwsd":
													ofstream tempFile("WaveSound/" + files[i].fileName, ofstream::binary);
													break;
													case "bcwar":
													ofstream tempFile("WaveArchive/" + files[i].fileName, ofstream::binary);
													break;
													case "bcgrp":
													ofstream tempFile("SoundGroup/" + files[i].fileName, ofstream::binary);
													break;
													case "bcbnk":
													ofstream tempFile("SoundBank/" + files[i].fileName, ofstream::binary);
													}
													*/				ofstream tempFile(files[i].fileName, ofstream::binary);
									buffer = (char*)malloc((int)files[i].size + 1);
									fread(buffer, 1, (int)files[i].size, sar);
									tempFile.write(buffer, (int)files[i].size);
									tempFile.close();
									printf("File %d written\n", i);
								}
								else
									continue;
							}

						}

						fclose(sar);
						return 0;
					end:
						fclose(sar);
						return 1;
					}
				
			