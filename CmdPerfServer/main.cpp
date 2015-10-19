#include <SDKDDKVer.h>
#include <Windows.h>
#include <conio.h>	// _kbhit
#include <stdio.h>
#include <tchar.h>
#define NE_USE_PERF 1
#include "server.h"

// main.cpp

void NTAPI Task_DoWork( PTP_CALLBACK_INSTANCE instance, PVOID context, PTP_WORK work )
{
	NePerfEnter("Work", __FUNCTION__, __FILE__, __LINE__);
	printf("[Worker] Working...\n");
	NePerfLeave();
}

void Game_Run()
{
	printf("[Game] Running...\n");
	PTP_WORK work[8] = {};
	while(!_kbhit())
	{
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

