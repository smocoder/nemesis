#include "server.h"
#include <SDKDDKVer.h>
#include <Windows.h>
#include <stdint.h>

namespace nemesis { namespace perf { namespace detail
{
	class Socket
	{
	public:
		Socket() 
			: Instance(INVALID_SOCKET)
		{}

	public:
		Socket Accept()
		{
			Socket remote;
			remote.Instance = accept( Instance, nullptr, nullptr );
			return remote;
		}
		bool Listen( int port )
		{
			Close();
			Instance = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
			if (!IsOpen())
				return false;
			sockaddr_in addr;
			addr.sin_family		 = AF_INET;
			addr.sin_addr.s_addr = htonl( 0 );
			addr.sin_port		 = htons( port );
			if (bind( Instance, (sockaddr*)&addr, sizeof(addr) ) < 0)
				return false;
			if (listen( Instance, SOMAXCONN ) < 0)
				return false;
			return true;
		}
		bool IsOpen() const
		{
			return (Instance != INVALID_SOCKET);
		}
		void Close()
		{
			closesocket(Instance);
			Instance = INVALID_SOCKET;
		}
		bool Send( const void* data, uint32_t size )
		{
			if (!IsOpen())
				return false;
			const char* pos = (const char*)data;
			const char* end = pos + size;
			while ( pos < end )
			{
				int written = send( Instance, pos, (int)(end-pos), 0 );
				if (written > 0)
					pos += written;
				else
					return false;
			}
			return true;
		}
	private:
		SOCKET Instance;
	};

	class Semaphore
	{
	public:
		Semaphore()										{ Handle = nullptr; }
		~Semaphore()									{ Close(); }
		void Create( uint32_t initial, uint32_t limit )	{ Close(); Handle = CreateSemaphoreA( nullptr, initial, limit, nullptr ); }
		void Close()									{ CloseHandle( Handle ); }
		void Signal( uint32_t count = 1 )				{ ReleaseSemaphore( Handle, count, nullptr ); }
		void Wait()										{ WaitForSingleObject( Handle, INFINITE ); }
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
		Thread()
			: Handle(nullptr)
			, Quit(0)
		{}

		~Thread()
		{ Stop(); }

	public:
		bool Continue() const 
		{ return !Quit; }

		void Start( void (*proc)( void* arg ), void* arg )
		{
			if (Handle)
				return;
		}
		void Stop() 
		{
			Quit = 1;
			WaitForSingleObject( Handle, INFINITE );
			Handle = nullptr;
		}

	private:
		HANDLE Handle;
		uint32_t Quit;
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
