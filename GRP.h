typedef struct {
	u32 id;
	u32 offset;
	u32 fileId;
	string name;
	string outfolder;
	u32 groupBank;
	bool bankExists;
	u32 groupWarc;
	u32 sectionCount;
	u32 infoOffset;
	u32 infoSize;
	u32 fileOffset;
	u32 fileSize;
	u32 infxOffset;
	u32 infxSize;
	u32 fileCount;
	u32 tempId;

	void processGroup(FILE * &csar) {
		fseek(csar, files[fileId].fileOffset, SEEK_SET);
//		printf("Checking group at %X\n", ftell(csar));
		typeHeader = ReadBE(csar, 32);
		if (typeHeader != 0x43475250)
			printf("Group might be external, because it isn't here.\n");
		else {
//			printf("Found group\n");
			fseek(csar, 0xC, SEEK_CUR);
			sectionCount = ReadLE(csar, 16);
			fseek(csar, 2, SEEK_CUR);
			for (j = 0; j < (int)sectionCount; j++) {
				//			printf("%X\n", ftell(sar));
				tempHeader = ReadLE(csar, 16);
				fseek(csar, 2, SEEK_CUR);
				if (tempHeader == 0x7800) {
					// Found INFO pointer
					infoOffset = ReadLE(csar, 32) + files[fileId].fileOffset;
//					printf("INFO section at %X\n", infoOffset);
					infoSize = ReadLE(csar, 32);
				}
				else if (tempHeader == 0x7801) {
					// Found FILE pointer
					fileOffset = ReadLE(csar, 32) + files[fileId].fileOffset;
					fileSize = ReadLE(csar, 32);
				}
				else if (tempHeader == 0x7802) {
					// Found INFX pointer
					infxOffset = ReadLE(csar, 32) + files[fileId].fileOffset;
					infxSize = ReadLE(csar, 32);
				}
				else {
					printf("Unrecognized section pointer\n");
					fclose(csar);
					exit(1);
				}
			}

			// Reading INFO section
			fseek(csar, infoOffset + 8, SEEK_SET);
			fileCount = ReadLE(csar, 32);
			printf("%d files for this group\n", fileCount);
			bankExists = false;
			for (m = 0; m < (int)fileCount; m++) {
				fseek(csar, infoOffset + 12 + m * 8, SEEK_SET);
				tempHeader = ReadLE(csar, 16);
				fseek(csar, 2, SEEK_CUR);
				if (tempHeader != 0x7900)
					continue;
				else {
					printf("Iteration %d - Currently at %X\n", m, ftell(csar));
					tempOffset = ReadLE(csar, 32) + infoOffset + 8;
					fseek(csar, tempOffset, SEEK_SET);
					printf("Now at %X\n", ftell(csar));
					tempId = ReadLE(csar, 32);
					printf("Found file %d\n", tempId);
					tempHeader = ReadLE(csar, 16);
					fseek(csar, 2, SEEK_CUR);
					if (tempHeader != 0x1F00)
						continue;
					else {
						files[tempId].fileOffset = ReadLE(csar, 32) + fileOffset + 8;
						fseek(csar, files[tempId].fileOffset, SEEK_SET);
						
						if (files[tempId].type == "bcwar") {
							warcs[files[tempId].id].processWarc(csar);
						}
						else
							continue;
					}
				}
			}

			// Looping through again, but this time focusing on banks
			for (m = 0; m < (int)fileCount; m++) {
				fseek(csar, infoOffset + 12 + m * 8, SEEK_SET);
				tempHeader = ReadLE(csar, 16);
				fseek(csar, 2, SEEK_CUR);
				if (tempHeader != 0x7900)
					continue;
				else {
					printf("Iteration %d - Currently at %X\n", m, ftell(csar));
					tempOffset = ReadLE(csar, 32) + infoOffset + 8;
					fseek(csar, tempOffset, SEEK_SET);
					printf("Now at %X\n", ftell(csar));
					tempId = ReadLE(csar, 32);
					printf("Found file %d\n", tempId);
					tempHeader = ReadLE(csar, 16);
					fseek(csar, 2, SEEK_CUR);
					if (tempHeader != 0x1F00)
						continue;
					else {
						files[tempId].fileOffset = ReadLE(csar, 32) + fileOffset + 8;
						fseek(csar, files[tempId].fileOffset, SEEK_SET);

						if (files[tempId].type == "bcbnk") {
							banks[files[tempId].id].processBank(csar);;
						}
						else
							continue;
						}
					}
				}
			

		}
	}
} group;