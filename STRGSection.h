//#include <sar.h>


typedef struct {
	u32 id;
	u32 offset;
	u32 size;
	u32 strgTableOffset;
	u32 strgCount;
	u32 otherTableOffset;
	vector<strg> names;
	void getStrings(FILE * &csar) {
		fseek(csar, offset, SEEK_SET);
		if (id = ReadBE(csar, 32) != 0x53545247) {
			printf("STRG section not found\n");
			fclose(csar);
			exit(1);
		}
		else {
			fseek(csar, offset + 8, SEEK_SET);
			for (i = 0; i < 2; i++)
			{
				tempHeader = ReadLE(csar, 16);
				fseek(csar, 2, SEEK_CUR);
				if (tempHeader == 0x2400)
					strgTableOffset = ReadLE(csar, 32); // Found string table pointer
				else if (tempHeader == 0x2401)
					otherTableOffset = ReadLE(csar, 32); // Found other table pointer
				else {
					printf("Unrecognized table pointer\n");
					fclose(csar);
					exit(1);
				}
			}
			strgCount = ReadLE(csar, 32);
			names.resize(strgCount);
			// Fetching string offsets...
			for (i = 0; i < int(strgCount); i++)
			{
				tempHeader = ReadLE(csar, 16);
				fseek(csar, 2, SEEK_CUR);
				if (tempHeader == 0x1f01) {
					// Found string pointer
					names[i].offset = ReadLE(csar, 32);
					names[i].size = ReadLE(csar, 32);
					//					names[i].name.resize(names[i].size);	
				}
				else {
					// String might be nulled
					continue;
				}
			}

			// Fetching actual strings
			for (i = 0; i < int(strgCount); i++)
			{
				fseek(csar, offset + 8 + strgTableOffset + names[i].offset, SEEK_SET);
				for (j = 0; j < int(names[i].size) - 1; j++) {
					names[i].name[j] = ReadLE(csar, 8);
					//					cout << names[i].name[j];
				}
				//				printf("%s\n", names[i].name);
			}
		}

	}
} strgSection;