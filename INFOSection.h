//#include <sar.h>

vector<audio> sounds;
vector<player> players;
vector<soundSet> sets;
vector<group> groups;

typedef struct {
	u32 id;
	u32 offset;
	u32 size;
	u32 audioTableOffset; // 0x2100
	u32 audioCount;
	u32 setTableOffset; // 0x2104
	u32 setCount;
	u32 bankTableOffset; // 0x2101
	u32 bankCount;
	u32 warcTableOffset; // 0x2103
	u32 warcCount;
	u32 groupTableOffset; // 0x2105
	u32 groupCount;
	u32 playerTableOffset; // 0x2102
	u32 playerCount;
	u32 fileTableOffset; // 0x2106
	u32 fileCount;
	u32 fileTableEnd; // 0x220B

	void processInfoTable(FILE * &csar) {
		fseek(csar, offset, SEEK_SET);
		if (id = ReadBE(csar, 32) != 0x494E464F) {
			printf("INFO section not found\n");
			fclose(csar);
			exit(1);
		}

		else {
			fseek(csar, offset + 8, SEEK_SET);
			for (i = 0; i < 8; i++)
			{
				tempHeader = ReadLE(csar, 16);
				fseek(csar, 2, SEEK_CUR);
				switch (tempHeader) {
				case 0x2100:
					audioTableOffset = ReadLE(csar, 32); // Found audio table pointer
					break;
				case 0x2101:
					bankTableOffset = ReadLE(csar, 32); // Found bank table pointer
					break;
				case 0x2102:
					playerTableOffset = ReadLE(csar, 32); // Found player table pointer
					break;
				case 0x2103:
					warcTableOffset = ReadLE(csar, 32); // Found wave archive table pointer
					break;
				case 0x2104:
					setTableOffset = ReadLE(csar, 32); // Found WSD/SEQ set table pointer
					break;
				case 0x2105:
					groupTableOffset = ReadLE(csar, 32); // Found group table pointer
					break;
				case 0x2106:
					fileTableOffset = ReadLE(csar, 32); // Found file table pointer
					break;
				case 0x220B:
					fileTableEnd = ReadLE(csar, 32); // Found end of file table
					printf("File table ends at %X\n", offset + 8 + fileTableEnd);
					break;
				default:
					printf("Unrecognized table pointer\n");
					fclose(csar);
					exit(1);
				}
			}
		}
	}
	
	void readBankTable(FILE * &csar) {
		fseek(csar, offset + 8 + bankTableOffset, SEEK_SET);
		bankCount = ReadLE(csar, 32);
		banks.resize(bankCount);
		for (i = 0; i < (int)bankCount; i++) {
			banks[i].id = i;
			tempHeader = ReadLE(csar, 16);
			fseek(csar, 2, SEEK_CUR);
			if (tempHeader == 0x2206) {
				banks[i].offset = ReadLE(csar, 32);
				//		printf("Bank %d info located at %X\n", i, infoSec.offset + 8 + infoSec.bankTableOffset + banks[i].offset);
			}
			else {
				fseek(csar, 4, SEEK_CUR);
			}
		}
	}

	void readFileTable(FILE * &csar) {
		fseek(csar, offset + 8 + fileTableOffset, SEEK_SET);
		fileCount = ReadLE(csar, 32);
		files.resize(fileCount);
		// Getting info pointers for files
		for (i = 0; i < (int)fileCount; i++) {
			tempHeader = ReadLE(csar, 16);
			fseek(csar, 2, SEEK_CUR);
			if (tempHeader == 0x220A)
				files[i].offset = ReadLE(csar, 32);
			else {
				fseek(csar, 4, SEEK_CUR);
			}
		}
	}

} infoSection;