#include <string>
#include <vector>
using namespace std;


#define INTR_FREQUENCY (1.0/192.0)


/* Structures for Sound Archives and their sub-sections */

typedef   signed char      s8;
typedef unsigned char      u8;
typedef   signed short     s16;
typedef unsigned short     u16;
typedef   signed int       s32;
typedef unsigned int       u32;

u16 tempHeader;
u32 typeHeader;
u32 tempOffset;
u8 tempChar;
char * buffer;
int i, j, k, l, m;



static inline u32 ReadLE(FILE *f, u32 b) {
	u32 v = 0;
	for (u32 i = 0; i<b; i += 8) v |= fgetc(f) << i;
	return v;
}

static inline u32 ReadBE(FILE *f, s32 b) {
	u32 v = 0;
	for (s32 i = b - 8; i >= 0; i -= 8) v |= fgetc(f) << i;
	return v;
}


double LogB( double n, double b )  
{  
    // log(n)/log(2) is log2.  
    return log( n ) / log( b );  

}

double timeToTimecents(double time){
	double timeCent = floor(1200 * LogB(time, 2));
	if (timeCent > -12000 && timeCent < 0)
		return timeCent + 65536;
	else if (timeCent < -12000)
		return -12000;
	else
		return timeCent;
}

const u16 sustainLevTable[] = { 0xFD2D, 0xFD2E, 0xFD2F, 0xFD75, 0xFDA7, 0xFDCE, 0xFDEE, 0xFE09, 0xFE20, 0xFE34, 0xFE46, 0xFE57, 0xFE66, 0xFE74,
0xFE81, 0xFE8D, 0xFE98, 0xFEA3, 0xFEAD, 0xFEB6, 0xFEBF, 0xFEC7, 0xFECF, 0xFED7, 0xFEDF, 0xFEE6, 0xFEEC, 0xFEF3,
0xFEF9, 0xFEFF, 0xFF05, 0xFF0B, 0xFF11, 0xFF16, 0xFF1B, 0xFF20, 0xFF25, 0xFF2A, 0xFF2E, 0xFF33, 0xFF37, 0xFF3C,
0xFF40, 0xFF44, 0xFF48, 0xFF4C, 0xFF50, 0xFF53, 0xFF57, 0xFF5B, 0xFF5E, 0xFF62, 0xFF65, 0xFF68, 0xFF6B, 0xFF6F,
0xFF72, 0xFF75, 0xFF78, 0xFF7B, 0xFF7E, 0xFF81, 0xFF83, 0xFF86, 0xFF89, 0xFF8C, 0xFF8E, 0xFF91, 0xFF93, 0xFF96,
0xFF99, 0xFF9B, 0xFF9D, 0xFFA0, 0xFFA2, 0xFFA5, 0xFFA7, 0xFFA9, 0xFFAB, 0xFFAE, 0xFFB0, 0xFFB2, 0xFFB4, 0xFFB6,
0xFFB8, 0xFFBA, 0xFFBC, 0xFFBE, 0xFFC0, 0xFFC2, 0xFFC4, 0xFFC6, 0xFFC8, 0xFFCA, 0xFFCC, 0xFFCE, 0xFFCF, 0xFFD1,
0xFFD3, 0xFFD5, 0xFFD6, 0xFFD8, 0xFFDA, 0xFFDC, 0xFFDD, 0xFFDF, 0xFFE1, 0xFFE2, 0xFFE4, 0xFFE5, 0xFFE7, 0xFFE9,
0xFFEA, 0xFFEC, 0xFFED, 0xFFEF, 0xFFF0, 0xFFF2, 0xFFF3, 0xFFF5, 0xFFF6, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFC, 0xFFFD,
0xFFFF, 0x0000 };

double attackTable[] = {13122, 6546, 4356, 3261, 2604, 2163, 1851, 1617, 1434, 1287, 1167, 1068, 984, 912, 849, 795, 747, 702, 666, 630, 600, 570, 543, 519, 498, 477, 459, 441, 426, 411, 396, 384, 372, 360, 348, 336, 327, 318, 309, 300, 294, 285, 279, 270, 264, 258, 252, 246, 240, 234, 231, 225, 219, 216, 210, 207, 201, 198, 195, 192, 186, 183, 180, 177, 174, 171, 168, 165, 162, 159, 156, 153.5, 153, 150, 147, 144, 141.5, 141, 138, 135.5, 135, 132, 129.5, 129, 126, 123.5, 123, 120.5, 120, 117, 114.5, 114, 111.5, 111, 108.5, 108, 105.7, 105.35, 105, 102.5, 102, 99.5, 99, 96.7, 96.35, 96, 93.5, 93, 90, 87, 81, 75, 72, 69, 63, 60, 54, 48, 45, 39, 36, 30, 24, 21, 15, 12, 9, 6.1e-6};
double holdTable[] = {6e-6, 1, 2, 4, 6, 9, 12, 16, 20, 25, 30, 36, 42, 49, 56, 64, 72, 81, 90, 100, 110, 121, 132, 144, 156, 169, 182, 196, 210, 225, 240, 256, 272, 289, 306, 324, 342, 361, 380, 400, 420, 441, 462, 484, 506, 529, 552, 576, 600, 625, 650, 676, 702, 729, 756, 784, 812, 841, 870, 900, 930, 961, 992, 1024, 1056, 1089, 1122, 1156, 1190, 1225, 1260, 1296, 1332, 1369, 1406, 1444, 1482, 1521, 1560, 1600, 1640, 1681, 1722, 1764, 1806, 1849, 1892, 1936, 1980, 2025, 2070, 2116, 2162, 2209, 2256, 2304, 2352, 2401, 2450, 2500, 2550, 2601, 2652, 2704, 2756, 2809, 2862, 2916, 2970, 3025, 3080, 3136, 3192, 3249, 3306, 3364, 3422, 3481, 3540, 3600, 3660, 3721, 3782, 3844, 3906, 3969, 4032, 4096};
double decayTable[] = { -0.00016, -0.00047, -0.00078, -0.00109, -0.00141, -0.00172, -0.00203, -0.00234, -0.00266, -0.00297, -0.00328, -0.00359, -0.00391, -0.00422, -0.00453, -0.00484, -0.00516, -0.00547, -0.00578, -0.00609, -0.00641, -0.00672, -0.00703, -0.00734, -0.00766, -0.00797, -0.00828, -0.00859, -0.00891, -0.00922, -0.00953, -0.00984, -0.01016, -0.01047, -0.01078, -0.01109, -0.01141, -0.01172, -0.01203, -0.01234, -0.01266, -0.01297, -0.01328, -0.01359, -0.01391, -0.01422, -0.01453, -0.01484, -0.01516, -0.01547, -0.01579, -0.016, -0.01622, -0.01644, -0.01667, -0.0169, -0.01714, -0.01739, -0.01765, -0.01791, -0.01818, -0.01846, -0.01875, -0.01905, -0.01935, -0.01967, -0.02, -0.02034, -0.02069, -0.02105, -0.02143, -0.02182, -0.02222, -0.02264, -0.02308, -0.02353, -0.024, -0.02449, -0.025, -0.02553, -0.02609, -0.02667, -0.02727, -0.02791, -0.02857, -0.02927, -0.03, -0.03077, -0.03158, -0.03243, -0.03333, -0.03429, -0.03529, -0.03636, -0.0375, -0.03871, -0.04, -0.04138, -0.04286, -0.04444, -0.04615, -0.048, -0.05, -0.05217, -0.05455, -0.05714, -0.06, -0.06316, -0.06667, -0.07059, -0.075, -0.08, -0.08571, -0.09231, -1, -0.10909, -0.12, -0.13333, -0.15, -0.17143, -2, -2.4, -3, -4, -6, -12, -24, -65535 };


typedef struct {
	u32 id; 
	u16 order;
	u16 headerSize;
	u32 fileSize;
	u32 sectionCount;
//	u32 strgOffset; // 0x2000
//	u32 infoOffset; // 0x2001
//	u32 fileOffset; // 0x2002
} csarHeader;

typedef struct {
	u32 offset;
	u32 size;
	char name[100];
} strg;

typedef struct {
	u32 offset;
	u32 fileId;
	u32 strgId;
	u32 bank;
	string name;
} audio;

// Placing noteRegion first so Instrument knows the struct type



typedef struct {
	u32 id;
	u32 offset;
} player;


typedef struct {
	u32 id;
	u32 offset;
	u32 fileId;
	u32 strgId;
	string name;
} soundSet;

typedef struct {
	int id;
	u32 offset;
	string type;
	bool internalFile;
	u32 fileOffset;
	u32 fileInfoLength;
	u32 size;
	u32 bank;
	string fileName;
	u32 absoluteOffset;
	char fileLocation[100];
} sarFile;

vector<sarFile> files;

typedef struct {
	u32 id;
	u32 offset;
	u32 size;
} fileSection;



u16 GetFallingRate(u8 DecayTime)
{
	u32 realDecay;
	if (DecayTime == 0x7F)
		realDecay = 0xFFFF;
	else if (DecayTime == 0x7E)
		realDecay = 0x3C00;
	else if (DecayTime < 0x32)
	{
		realDecay = DecayTime * 2;
		++realDecay;
		realDecay &= 0xFFFF;
	}
	else
	{
		realDecay = 0x1E00;
		DecayTime = 0x7E - DecayTime;
		realDecay /= DecayTime;		//there is a whole subroutine that seems to resolve simply to this.  I have tested all cases
		realDecay &= 0xFFFF;
	}
	return (u16)realDecay;
}

/*

double decayTimes[] = {462714, 154236, 92538, 66099, 51408, 42060, 35589, 30843, 27213, 24348, 22029, 20115, 18504, 17133, 15951, 14922, 14016, 13215, 12501, 11859, 11280, 10755, 10278, 9840, 9438, 9069, 8727, 8409, 8112, 7839, 7581, 7341, 7113, 6903, 6702, 6513, 6333, 6165, 6006, 5853, 5709, 5571, 5439, 5313, 5196, 5079, 4971, 4866, 4767, 4668, 4575, 4515, 4455, 4395, 4332, 4272, 4212, 4152, 4092, 4032, 3972, 3912, 3852, 3792, 3732, 3672, 3609, 3549, 3489, 3429, 3369, 3309, 3249, 3189, 3129, 3069, 3009, 2949, 2886, 2826, 2766, 2706, 2646, 2586, 2526, 2466, 2406, 2346, 2286, 2226, 2163, 2103, 2043, 1983, 1923, 1863, 1803, 1743, 1683, 1623, 1563, 1503, 1440, 1380, 1320, 1260, 1200, 1140, 1080, 1020, 960, 900, 840, 780, 717, 657, 597, 537, 477, 417, 357, 297, 237, 177, 117, 57, 27, 6e-6};

*/



