#include <SDKDDKVer.h>
#include <Windows.h>
#include <conio.h>	// _kbhit
#include <stdio.h>
#include <tchar.h>
#define NE_USE_PERF 1
#include "server.h"

class StopWatch
{
public:
	StopWatch()
	{
		QueryPerformanceFrequency(&Freq);
		Start();
	}

public:
	void Start()
	{ QueryPerformanceCounter(&Tick); }

	LONGLONG ElapsedTicks() const
	{ 
		LARGE_INTEGER tick;
		QueryPerformanceCounter(&tick);
		return tick.QuadPart-Tick.QuadPart; 
	}

	LONGLONG ElapsedMs64() const
	{ return (ElapsedTicks() * 1000) / Freq.QuadPart; }

	DWORD ElapsedMs() const
	{ return (DWORD)ElapsedMs64(); }

	float ElapsedMsF() const
	{ return (float)(ElapsedTicks() * 1000) / (float)Freq.QuadPart; }

	float ElapsedSec() const
	{ return (float)ElapsedTicks() / (float)Freq.QuadPart; }

public:
	LARGE_INTEGER Freq;
	LARGE_INTEGER Tick;
};

void NTAPI Task_DoWork( PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work )
{
	NePerfEnter("Work", __FUNCTION__, __FILE__, __LINE__);
	printf("[Worker] Working...\n");
	Sleep(rand()%33);
	NePerfLeave();
}

void Game_Run()
{
	StopWatch timer;
	srand(timer.Tick.LowPart);
	printf("[Game] Running...\n");
	PTP_WORK work[8] = {};
	while(!_kbhit())
	{
		timer.Start();
		{
			NePerfEnter( "Main Loop", __FUNCTION__, __FILE__, __LINE__ );
			printf("[Game] Begin of frame...\n");
			for ( int i = 0; i < _countof(work); ++i )
				work[i] = CreateThreadpoolWork( &Task_DoWork, nullptr, nullptr );
			for ( int i = 0; i < _countof(work); ++i )
				SubmitThreadpoolWork( work[i] );
			for ( int i = 0; i < _countof(work); ++i )
				WaitForThreadpoolWorkCallbacks( work[i], FALSE );
			printf("[Game] End of frame...\n");
			NePerfLeave();
		}
		{
			NePerfEnter( "Throttle", __FUNCTION__, __FILE__, __LINE__ );
			while (timer.ElapsedMs() < 33)
				Sleep(0);
			NePerfLeave();
		}
		nemesis::perf::Server_NextFrame();
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	{
		printf("[Server] Initializing...\n");
		NePerfInitialize();
	}
	{
		printf("[Server] Starting sender on tcp port 16001...\n");
		NePerfStart( 16001 );
	}
	{
		Game_Run();
	}
	{
		printf("[Server] Stopping sender...\n");
		NePerfStop();
	}
	{
		printf("[Server] Shutting down...\n");
		NePerfShutdown();
	}
	return 0;
}

