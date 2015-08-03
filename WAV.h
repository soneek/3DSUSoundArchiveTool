typedef struct {
	u32 id;
	u32 offset;
	u32 size;
	u16 sectionCount;
	u32 infoOffset;
	u32 infoLength;
	u32 dataOffset;
	u32 dataLength;
	u8 type; // 0 - PCM8, 1 - PCM16, 2 - DSP ADPCM
	u8 loopFlag;
	u32 sampleRate;
	u32 loopStart;
	u32 loopEnd;
	u32 channels;
	u16 adpcmCoeff[16];
} bwav;