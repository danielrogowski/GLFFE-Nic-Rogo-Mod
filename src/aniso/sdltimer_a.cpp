#include <SDL_timer.h>
#include "../common/ffeapi.h"
#include "../common/ffecfg.h"

extern int SDLMsgHandler (void);

static ULONG tmStamp = 0;
static ULONG tmLastClock = 0;
static int tmInit = 0;


extern "C" void TimerFrameUpdate (void)
{
	SDLMsgHandler();
	/*
	TimerGetTimeStamp ();
	while (!SDLMsgHandler ()) SDL_Delay (100);
	tmLastClock = SDL_GetTicks ();
	*/
}

extern "C" ULONG TimerGetTimeStamp (void)
{
	ULONG curTime;

	curTime = SDL_GetTicks ();
	while (tmLastClock + 5 <= curTime)
	{
		tmStamp += 5;
		tmLastClock += 5;
	}

	return tmStamp;
}

void TimerSleep (void)
{
	SDL_Delay (1);
}

void TimerInit (void)
{
	tmLastClock = SDL_GetTicks ();
}

void TimerCleanup (void)
{
}

extern "C" void TimerLockData (void* data, long size)
{
	// hmm
}

extern "C" void TimerUnlockData (void* data, long size)
{
	// hmm
}

extern "C" void TimerCli (void)
{
	// hmm
}

extern "C" void TimerSti (void)
{
	// hmm
}

long			asmhandler;
SDL_TimerID		timerID;
typedef void	*asmhandler_type(void);

Uint32 timerhandler(Uint32 time, void * param) {
	
	((asmhandler_type*)asmhandler)();
	return time;
}

extern "C" void TimerInstallCallback (long handler, long frequency)
{
	asmhandler = handler;
	timerID = SDL_AddTimer(1000/frequency,(SDL_NewTimerCallback) timerhandler, NULL);
}

extern "C" void TimerRemoveCallback (long handler)
{
	if(handler==asmhandler) SDL_RemoveTimer(timerID);
}

