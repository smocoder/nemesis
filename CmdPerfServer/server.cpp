#include "server.h"
#include <SDKDDKVer.h>
#include <Windows.h>
#include <stdint.h>

namespace nemesis { namespace perf { namespace detail
{
	class Socket
	{
	public:
		Socket();
	public:
		Socket Accept();
		bool Listen( int port );
		bool IsOpen() const;
		void Close();
		bool Send( const void* data, uint32_t size );
	private:
	};

	class Semaphore
	{
	public:
		Semaphore()										{ Handle = nullptr; }
		~Semaphore()									{ Close(); }
		void Create( uint32_t initial, uint32_t limit )	{ Close(); Handle = CreateSemaphoreA( nullptr, initial, limit, nullptr ); }
		void Close()									{ CloseHandle( Handle ); }
		void Signal( uint32_t count = 1 )				{ ReleaseSemaphore( Handle, count, nullptr ); }
		void Wait()										{ WaitForSingleObject( Handle, 0xffffffff ); }
	private:
		HANDLE Handle;
	};

	class CriticalSection
	{
	public:
		CriticalSection()	{ InitializeCriticalSection( &Instance ); }
		~CriticalSection()	{ DeleteCriticalSection( &Instance ); }
		void Enter()		{ EnterCriticalSection( &Instance ); }
		void Leave()		{ LeaveCriticalSection( &Instance ); }
	private:
		CRITICAL_SECTION Instance;
	};

	class Thread
	{
	public:
		Thread();
		~Thread();
	public:
		bool Continue() const;
		void Start( void (*proc)( void* arg ), void* arg );
		void Stop();
	private:
	};

	struct Buffer
	{
		enum { NUM_BYTES = 8 * 64 * 1024 };
		uint32_t Size;
		uint8_t Data[ NUM_BYTES ];
	};

	class Queue
	{
	public:
		Queue()
			: ReadPos(0)
			, WritePos(0)
		{
			Space.Create( NUM_ITEMS, NUM_ITEMS );
			Work .Create(		  0, NUM_ITEMS );
		}
		~Queue()
		{
			Close();
		}
	public:
		void Push( const Buffer& s )
		{
			Space.Wait();
			{
				Mutex.Enter();
				{
					Buffer& d = Item[WritePos];
					WritePos = (WritePos+1) % NUM_ITEMS;
					d.Size = s.Size;
					memcpy( d.Data, s.Data, s.Size );
				}
				Mutex.Leave();
			}
			Work.Signal();
		}
		void Pop( Buffer& d )
		{
			Work.Wait();
			{
				Mutex.Enter();
				{
					const Buffer& s = Item[ReadPos];
					ReadPos = (ReadPos+1) % NUM_ITEMS;
					d.Size = s.Size;
					memcpy( d.Data, s.Data, s.Size );
				}
				Mutex.Leave();
			}
			Space.Signal();
		}
		void Close()
		{
			Space.Signal( NUM_ITEMS );
			Work .Signal( NUM_ITEMS );
		}
		void Open()
		{
			ReadPos = WritePos = 0;
		}
	private:
		enum { NUM_ITEMS = 8 };
		Semaphore Work;
		Semaphore Space;
		int32_t ReadPos;
		int32_t WritePos;
		CriticalSection Mutex;
		Buffer Item[NUM_ITEMS];
	};

	class Sender
	{
	public:
		Sender()	{}
		~Sender()	{ Stop(); }
	public:
		void Start( int port )
		{
			if (Server.IsOpen())
				return;
			if (!Server.Listen( port ))
				return;
			Worker.Start( Run, this );
		}
		void Stop()
		{
			Client.Close();
			Server.Close();
			Queue.Close();
			Worker.Stop();
		}
		void Send( const Buffer& data )
		{
			if (!Client.IsOpen())
				return;
			Queue.Push( data );
		}
	private:
		static void Run( void* arg )
		{ ((Sender*)arg)->Accept(); }

		void Accept()
		{
			for ( ; Worker.Continue() ; )
			{
				Client = Server.Accept();
				if (!Client.IsOpen())
					continue;
				{
					Queue.Open();
					Publish();
					Client.Close();
				}
			}
		}
		void Publish()
		{
			Buffer buffer;
			for ( ; Worker.Continue() ; )
			{
				Queue.Pop( buffer );
				if (!buffer.Size)
					continue;
				if (Client.Send( buffer.Data, buffer.Size ))
					continue;
				break;
			}
		}
	private:
		Queue Queue;
		Socket Server;
		Socket Client;
		Thread Worker;
	};

	class Recorder
	{
	};

	class Server
	{
	public:
		void Initialize();
		void Shutdown();
		void StartSender( int port );
		void StopSender();
		void NextFrame();
		void EnterScope( const char* tag, const char* func, const char* file, int line );
		void LeaveScope();
	private:
		Sender   Sender;
		Recorder Recorder;
	};

} } }

namespace nemesis
{
	perf::detail::Server PerfServer;

	void perf::Server_Initialize()
	{ PerfServer.Initialize(); }

	void perf::Server_Shutdown()
	{ PerfServer.Shutdown(); }

	void perf::Server_StartSender( int port )
	{ PerfServer.StartSender( port ); }

	void perf::Server_StopSender()
	{ PerfServer.StopSender(); }

	void perf::Server_NextFrame()
	{ PerfServer.NextFrame(); }

	void perf::Server_EnterScope( const char* tag, const char* func, const char* file, int line )
	{ PerfServer.EnterScope( tag, func, file, line ); }

	void perf::Server_LeaveScope()
	{ PerfServer.LeaveScope(); }
}
