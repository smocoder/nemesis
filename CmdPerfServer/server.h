namespace nemesis { namespace perf
{
	void Server_Initialize();
	void Server_Shutdown();
	void Server_StartSender( int port );
	void Server_StopSender();
	void Server_NextFrame();
	void Server_EnterScope( const char* tag, const char* func, const char* file, int line );
	void Server_LeaveScope();
} }

#if NE_USE_PERF
#	define NePerfInitialize	nemesis::perf::Server_Initialize
#	define NePerfShutdown	nemesis::perf::Server_Shutdown
#	define NePerfStart		nemesis::perf::Server_StartSender
#	define NePerfStop		nemesis::perf::Server_StopSender
#	define NePerfNextFrame	nemesis::perf::Server_NextFrame
#	define NePerfThread		nemesis::perf::Server_SetThreadInfo
#	define NePerfEnter		nemesis::perf::Server_EnterScope
#	define NePerfLeave		nemesis::perf::Server_LeaveScope
#else
#	define NePerfInitialize(...)
#	define NePerfShutdown(...)	
#	define NePerfStart(...)		
#	define NePerfStop(...)		
#	define NePerfNextFrame(...)	
#	define NePerfThread(...)		
#	define NePerfEnter(...)		
#	define NePerfLeave(...)		
#endif
