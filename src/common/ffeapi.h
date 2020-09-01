#ifndef __FFEAPI__H__
#define __FFEAPI__H__

// Nick
#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __CONFIGFILE__ "glffe.cfg"

/////////////////////////
// console replacement //
/////////////////////////
struct sMouseClickRemap {
	int top;
	int left;
	int bottom;
	int right;
	int to_x;
	int to_y;
};
struct sBmpPosition {
	int top;
	int left;
	int bottom;
	int right;
};
struct sBmpReplacement {
	int nTextureIndex;		// index of console texture
	int nMode;				// -2: true opaque; -1: true transparent; 0x0..0xFFF: 12 bit color monochrome transparent
	int nSrcPosIndex;		// index of structure of position in source texture
	int nDstPosIndex;		// index of structure of position on screen
};


typedef unsigned char	UCHAR;
typedef unsigned short	USHORT;
typedef short			SHORT;
typedef unsigned long	ULONG;
typedef unsigned int	UDWORD;

void SystemInit (void);
void SystemCleanup (void);

void VideoBlit (UCHAR *pData, long x, long y, long w, long h, long jump);
void VideoMaskedBlit (UCHAR *pData, long x, long y, long w, long h, long jump);
void VideoReverseBlit (UCHAR *pData, long x, long y, long w, long h, long jump);
void VideoGetPalValue (long palindex, UCHAR *pVal);
void VideoSetPalValue (long palindex, UCHAR *pVal);
long VideoPointerExclusive (void);
void VideoPointerEnable (void);
void VideoPointerDisable (void);

extern "C" USHORT SDLTextureColors[256];

long InputMouseReadButtons (void);
void InputMouseReadMickeys (long *pXMick, long *pYMick);
void InputMouseReadPos (long *pXPos, long *pYPos);

void InputJoyReadPos (long *pXPos, long *pYPos);
long InputJoyReadButtons (void);

void InputKeybReadStates (UCHAR *pKeyArray);
long InputKeybGetLastKey (void);
void InputKeybSetLastKey (long);

ULONG TimerGetTimeStamp (void);
void TimerSleep (void);
void TimerFrameUpdate (void);

void SoundCheckInit (long *pAll, long *pDigi, long *pMidi);
void SoundPlaySong (long index);
void SoundStopSong (void);
long SoundSongDone (void);
void SoundStopAllSamples (void);
void SoundPlaySample (long index, long vol, long pitch);

long SoundStreamGetUsedBlocks (void);
void SoundStreamReset (void);
void SoundStreamAddBlock (void *pData, long size);
void SoundStreamStart (void);
void SoundStreamWait (void);
void SoundStreamStop (void);

typedef struct {
	char pName[16];
	int type;
	int size;
} FileInfo;

char *DirMakeAVIName (char *pBuf, char *pStub);
char *DirMakeSampleName (char *pBuf, char *pFilename);
char *DirMakeSongName (char *pBuf, char *pFilename);
char *DirMakeCmmdrName (char *pBuf, char *pFilename);
char *DirGetCmmdrPath (void);
void DirResetCmmdrPath (void);
void DirNavigateTree (FileInfo *pFile);
int DirFindFirst (FileInfo *pFile);
int DirFindNext (FileInfo *pFile);

// Nick
void NickMain();
extern "C" BYTE PlanetDetailHigh;

#ifdef __cplusplus
}
#endif

#endif /* __FFEAPI_H__ */
