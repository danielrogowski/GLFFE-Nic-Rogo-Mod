//
// this is the "brute force" way of feeding jj's audio code to sdl_mixer 
// and added music support
//

#include <SDL.h>
#include <SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "ffeapi.h"
#include "ffecfg.h"
#include "io.h"

typedef struct {
	char pName[16];
	int loopcount;
	int pitchmod;
	int voice;
	int flags;
	Mix_Chunk* raw_chunk;
} FFESample;

const int soundNumSamples = 0x28;
#define SOUNDNUMSAMPLES 0x28

FFESample pSoundSampleDescs[SOUNDNUMSAMPLES] = {
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },		// 0x4
	{ "shipexp", 0, 0, -1, 0 ,NULL },
	{ "missexpl", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },	// 0x8
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "laser1", 0, 0, -1, 0 ,NULL },
	{ "laser2", 0, 0, -1, 0 ,NULL },
	{ "airopen", 0, 0, -1, 0 ,NULL },		// 0xc
	{ "airopen", 0, 0, -1, 0 ,NULL },
	{ "wind", 0, -0x4000, 8, 0 ,NULL },
	{ "station", 0x32, 0, 9, 0 ,NULL },
	{ "station", 0x32, 0x1000, 2, 0 ,NULL },	// 0x10
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "airopen", 0, 0, -1, 0 ,NULL },
	{ "airopen", 0, 0, -1, 0 ,NULL },
	{ "bip", 0, 0, -1, 0 ,NULL },			// 0x14
	{ "bip", 0, 0, -1, 0 ,NULL },
	{ "bong", 0, 0, -1, 0 ,NULL },
	{ "chime", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },		// 0x18
	{ "ecm", 2, 0, 3, 0 ,NULL },
	{ "necm", 2, 0, 4, 0 ,NULL },
	{ "shipexp", 0, 0, -1, 0 ,NULL },
	{ "missexpl", 0, 0, -1, 0 ,NULL },	// 0x1c
	{ "damage", 0, 0, -1, 0 ,NULL },
	{ "siren", 3, 0, 5, 0 ,NULL },
	{ "boom", 0, 0, 6, 0 ,NULL },
	{ "launch", 0, 0, -1, 0 ,NULL },		// 0x20
	{ "hype", 0, 0, -1, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "laser3", 0, 0, -1, 0 ,NULL },
	{ "laser4", 0, 0, -1, 0 ,NULL },		// 0x24
	{ "wind", 0, -0x4000, 7, 0 ,NULL },
	{ "nowt", 0, 0, -1, 0 ,NULL },
	{ "camera", 0, 0, -1, 0 ,NULL }
	};

const int soundNumSongs = 17;
#define SOUNDNUMSONGS 17

char ppSoundSongNames[SOUNDNUMSONGS][16] = {
	{ "qqfront" },
	{ "qqatmosp" },
	{ "qqtravel" },
	{ "qqdrama" },
	{ "qqsuspen" },
	{ "qqrock" },
	{ "qqescape" },
	{ "qqparadi" },
	{ "qqfront2" },
	{ "hallking" },
	{ "intro" },
	{ "babayaga" },
	{ "barem" },
	{ "jupalt" },
	{ "valkries" },
	{ "bluedan" },
	{ "ggofkiev" } };



struct SoundChannel
{
	int index;
	void *pData;
	int len;
	int vol;

	int loop;
	int pos;			
	int step;			// Both 24.8 fixed point
};

static void *ppSampleData[SOUNDNUMSAMPLES];	// Data (22050, S16)
static int pSampleLen[SOUNDNUMSAMPLES];		// length in samples (*2)

#define NUMCHANNELS 16

// 0 reserved for music, 1 reserved for video
static SoundChannel pChannel[NUMCHANNELS];

static int soundInit = 0;
static int buffersize = 4096;
int musicVolume = MIX_MAX_VOLUME;

#define BUFFERSIZE 4096

int pAccum[BUFFERSIZE];

int LoadSample (int index);
void FillSound (int channel);

/////////////////
// MAINTENANCE //
/////////////////

Mix_Chunk* raw_stream; 

void SoundInit ()
{
	memset(pAccum,0,sizeof(int)*BUFFERSIZE);
	CfgStruct cfg;
	CfgOpen (&cfg, __CONFIGFILE__);
	CfgFindSection (&cfg, "SOUND");
	CfgGetKeyVal (&cfg, "buffersize", &buffersize);
	int musicvolumepercent;
	CfgGetKeyVal (&cfg, "musicvolume", &musicvolumepercent);
	if(musicvolumepercent<0) musicvolumepercent=0;
	if(musicvolumepercent>100) musicvolumepercent=100;
	CfgClose (&cfg);

	int i;
	for (i=0; i<SOUNDNUMSAMPLES; i++) LoadSample (i);
	
	int audio_rate = 44100;
	Uint16 audio_format = AUDIO_S16SYS;
	int audio_channels = 1;
	int audio_buffers = buffersize;
 
	if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0) {
		fprintf(stderr, "Unable to initialize audio: %s\n", Mix_GetError());
		soundInit=0;
		return;
	} else {
		soundInit=1;
	}

	if(! ( raw_stream = Mix_QuickLoad_RAW((Uint8*)pAccum,BUFFERSIZE) ) ) {
		fprintf( stderr, "Initial Mix_QuickLoad_RAW: %s\n", Mix_GetError() );
	}
    
	Mix_ChannelFinished(FillSound);
	if( Mix_PlayChannel(0, raw_stream, 0) == -1 ){
		fprintf( stderr, "Inital Mix_PlaySound: %s\n", Mix_GetError() );
	}
	
	musicVolume = (int) ((float)musicvolumepercent/100.0f*(float)MIX_MAX_VOLUME) ;

}

void SoundCleanup ()
{
	SoundStopAllSamples();
	SoundStopSong();
	Mix_CloseAudio ();

	SoundStopAllSamples();
	SoundStreamStop();
	SoundStopSong();

	if(raw_stream!=NULL) {
		Mix_FreeChunk(raw_stream);
	}

	int i;
	for (i=0; i<SOUNDNUMSAMPLES; i++)
	{
		if(ppSampleData[i]) free (ppSampleData[i]);
		if(pSoundSampleDescs[i].raw_chunk != NULL) {
			Mix_FreeChunk( pSoundSampleDescs[i].raw_chunk );
			pSoundSampleDescs[i].raw_chunk = NULL;
		}
		ppSampleData[i] = NULL;
		pSampleLen[i] = 0;
	}
}

extern "C" void SoundCheckInit (long *pAll, long *pDigi, long *pMidi)
{
	*pAll = soundInit;
	*pDigi = soundInit;
	*pMidi = soundInit;
}


////////////
// SOUNDS //
////////////
extern "C" void SoundStopAllSamples (void)
{
	int i;
	for (i=1; i<NUMCHANNELS; i++) pChannel[i].pData = 0;
}

void FillSound (int channel)
{
	int i, n;
	int len = BUFFERSIZE;
	len /= 4;							// conv to bytes

	// Mix channels into accumulation buffer

	for (i=0; i<len; i++) pAccum[i] = 0;

	for (n=0; n<NUMCHANNELS; n++)
	{
		SoundChannel *pChan = pChannel + n;
		if (pChan->pData == 0) continue;
		Sint16 *pData = (Sint16 *)pChan->pData;

		for (i=0; i<len; i++)
		{
			if (pChan->vol > 250) pAccum[i] += pData[pChan->pos>>8];
			else pAccum[i] += (pData[pChan->pos>>8] * pChan->vol) >> 8;
			pChan->pos += pChan->step;

			if (pChan->pos >= pChan->len) {
				pChan->pos -= pChan->len;
				if (pChan->loop-- == 0) {
					pChan->pData = 0;
					break;
				}
			}
		}
	}

	// Clip to output

	for (i=0; i<len; i++)
	{
		if (pAccum[i] < -32768) pAccum[i] = -2147450880;
		else if (pAccum[i] > 0x7fff) pAccum[i] = 0x7fff7fff;
		else pAccum[i] = (pAccum[i] & 0xffff) | (pAccum[i] << 16);
	}

	Mix_FreeChunk(raw_stream);
	raw_stream = NULL;
	if(! ( raw_stream = Mix_QuickLoad_RAW((Uint8*)pAccum,BUFFERSIZE) ) ) {
		fprintf( stderr, "Mix_QuickLoad_RAW: %s\n", Mix_GetError() );
	}
	Mix_ChannelFinished(FillSound);
	if( Mix_PlayChannel(0, raw_stream, 0) == -1 ){
		fprintf( stderr, "Mix_PlaySound: %s\n", Mix_GetError() );
	}

}

extern "C" void SoundPlaySample (long index, long vol, long pitch)
{
	int fixednum, i;

	if (!soundInit) return;
	if (index >= soundNumSamples) return;
	if (ppSampleData[index] == NULL) return;

	if (pitch==-1) pitch = 22050;
	pitch += pSoundSampleDescs[index].pitchmod;
	if (pitch > 100000) pitch = 100000;
	if (pitch < 100) pitch = 100;

	// Find channel to play on

	fixednum = pSoundSampleDescs[index].voice;
	if (fixednum != -1)
	{
		SoundChannel *pChan = pChannel + fixednum;
		if (pChan->pData == 0) i = fixednum;
		else if (index != 16) return;
		else {
			// engine noise - modify frequency
			pChan->vol = vol >> 7;
			pChan->step = (pitch << 8) / 22050;
			pChan->loop = 0x32;
			return;
		}
	}
	else		// Find free or beam-laser channel
	{
		for (i=10; i<NUMCHANNELS; i++) {
			if (pChannel[i].pData == 0) break;
		}
		if (i == NUMCHANNELS) for (i=10; i<NUMCHANNELS; i++) {
			if (pChannel[i].index == 0xb) break;
		}
		if (i == NUMCHANNELS) return;		// no free slots found
	}

	// Play sample (index) in channel i

	SoundChannel *pChan = pChannel + i;
	pChan->index = index;
	pChan->len = pSampleLen[index] << 8;
	pChan->loop = pSoundSampleDescs[index].loopcount;
	pChan->pos = 0;
	pChan->step = (pitch << 8) / 22050;
	pChan->vol = vol >> 7;
	pChan->pData = ppSampleData[index];
}


int LoadSample (int index)
{
	char pBuf[256];
	FILE *pFile;
	int size = 0;
	void *pSample = NULL;

	DirMakeSampleName (pBuf, pSoundSampleDescs[index].pName);
	strcat (pBuf, ".raw");
	pFile = fopen (pBuf, "rb");
	if (pFile != NULL) 
	{
		fseek (pFile, 0, SEEK_END);
		size = ftell (pFile);
		fseek (pFile, 0, SEEK_SET);

		if (size != 0) {
			pSample = malloc (size);
			fread (pSample, 1, size, pFile);
			fclose (pFile);
		}
	}
	else fprintf (stderr, "Sample %s not found\n", pBuf);

	ppSampleData[index] = pSample;
	pSampleLen[index] = size / 2;
	return 1;
}



///////////////////////////
// VIDEO (TALKING HEADS) // 
///////////////////////////

static UCHAR pStreamData[22050*2*20];
static int pBlockEnd[1000];
static int lastblock = -1;
static int writepos = 0;

extern "C" void SoundStreamReset (void)
{
	if (soundInit == 0) return;
	if (pChannel[1].pData != NULL) SoundStreamStop();
	writepos = 0;
}

extern "C" void SoundStreamAddBlock (void *pData, long size)
{
	if (soundInit == 0) return;
	memcpy (pStreamData+writepos, pData, size);
	writepos += size;
	pBlockEnd[++lastblock] = writepos;
	pChannel[1].len = writepos << 7;
}

extern "C" long SoundStreamGetUsedBlocks (void)
{
	if (soundInit == 0) return 0;
	if (lastblock == -1) return 0;
	int curbuf = 0;
	int curpos = (pChannel[1].pos >> 8) * 2;
	while (pBlockEnd[curbuf] <= curpos) ++curbuf;
	return curbuf;
}

extern "C" void SoundStreamStart (void)
{
	if (soundInit == 0) return;
	pChannel[1].pos = 0;
	pChannel[1].step = 0x100;
	pChannel[1].vol = 255;
	pChannel[1].len = writepos << 7;
	pChannel[1].loop = 0;
	pChannel[1].pData = pStreamData;
}

static void *GetStreamPos ()
{
	return pChannel[1].pData;
}

extern "C" void SoundStreamWait (void)
{
	if (soundInit == 0) return;
	while (GetStreamPos () != NULL);
	SoundStreamStop();
}

extern "C" void SoundStreamStop (void)
{
	if (soundInit == 0) return;
	pChannel[1].pData = 0;
	lastblock = -1;
}

///////////
// MUSIC //
///////////

extern "C" void *load_hmp (void *buf, unsigned long insize, unsigned long *outsize);

static int ConvertHMP (char *pHmpName, char *pMidiName)
{
	FILE *pHmp, *pMidi;
	void *pHmpImage, *pMidiImage;
	unsigned long hmpsize, midisize;

	fprintf (stderr, "%s not found, converting from HMP version\n", pMidiName);
	pHmp = fopen (pHmpName, "rb");
	if (pHmp == 0) {
	fprintf (stderr, "%s not found, can't play music\n", pHmpName);
		return 0;
	}
	pMidi = fopen (pMidiName, "wb");
	if (pMidi == 0) {
	fprintf (stderr, "%s couldn't be opened for writing\n", pMidiName);
		fclose (pHmp);
		return 0;
	}
	
	fseek (pHmp, 0, SEEK_END);
	hmpsize = ftell (pHmp);
	fseek (pHmp, 0, SEEK_SET);
	pHmpImage = malloc (hmpsize);
	fread (pHmpImage, 1, hmpsize, pHmp);

	pMidiImage = load_hmp (pHmpImage, hmpsize, &midisize);
	fwrite (pMidiImage, 1, midisize, pMidi);

	fclose (pHmp);
	fclose (pMidi);
	free (pHmpImage);
	free (pMidiImage);
	return 1;
}


Mix_Music *music;
int musicPlaying = 0;

void musicFinished() {
	musicPlaying = 0;
}

char* extensions[4] = { ".ogg", ".mp3", ".mid", ".hmp" };

extern "C" void SoundPlaySong (long index) 
{	
	char pBuf[256], pBuf2[256], pBuf3[256];
	
	if(index<0) return;
	
	if(musicPlaying) {
		SoundStopSong();
	}
	
	DirMakeSongName (pBuf, ppSoundSongNames[index]);
	bool found = false;
	for(int i=0; i<3; i++) {
		sprintf(pBuf2,"%s%s",pBuf,extensions[i]);
		if (_access (pBuf2, 4) == 0) {
			if( (music = Mix_LoadMUS(pBuf2)) == NULL ) {
				fprintf (stderr, "failed to play %s: %s \n", pBuf2, Mix_GetError() );
			} else {
				found = true;
				break;
			}
		}
	}
	
	if(!found) {
		sprintf(pBuf3, "%s.hmp", pBuf);
		if (!ConvertHMP (pBuf3, pBuf2)) {
			return;
		} else {
			if( (music = Mix_LoadMUS(pBuf2)) == NULL ) {
				fprintf (stderr, "failed to play %s: %s \n", pBuf2, Mix_GetError() );
				return;
			} 
		}
	}

	musicPlaying = 1;
	Mix_HookMusicFinished(musicFinished);
	Mix_VolumeMusic(musicVolume);
	if(Mix_PlayMusic(music, 0) ==-1) {
		musicPlaying=0;
		return;
	}
}

extern "C" void SoundStopSong (void)
{
	if(musicPlaying) {
		Mix_HaltMusic();
		Mix_FreeMusic(music);
	}
	musicPlaying = 0;
}

extern "C" long SoundSongDone (void)
{
	return 0;
}
