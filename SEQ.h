typedef struct {
	u32 offset;
	u32 size;
	u32 end;
	int trackNo;
} seqTrack;

typedef struct {
	u32 trackLocationOffset;
	u32 trackCountOffset;
	u32 loopInfoOffset;
	int trackCount;
	vector<seqTrack> tracks;
}bseq;