#include "server.h"
#include <SDKDDKVer.h>
#include <Windows.h>
#include <stdint.h>
#include <atomic>
#include <vector>
#pragma comment (lib, "ws2_32.lib")

#define NeMakeFourCc( a, b, c, d ) ( ((a) << 24) | ((b) << 16) | ((c) << 8) | (d) )

namespace nemesis { namespace perf { namespace detail 
{
	namespace ping
	{
		struct Header
		{
			static const uint32_t ID   = NeMakeFourCc( 'N', 'E', 'M', ' ' );
			static const uint32_t TYPE = NeMakeFourCc( 'P', 'E', 'R', 'F' );

			uint32_t Id;
			uint32_t Type;

			bool IsValid() const
			{ return (Id == ID) && (Type == TYPE); }

			void Init()
			{
				Id = ID;
				Type = TYPE;
			}
		};

		struct Response
		{
			Header	 Header;
			uint32_t Data[4];
		};
	}

	namespace packet
	{
		struct Header
		{
			uint32_t Id;
			uint32_t Size;
		};

		struct Connect
		{
			static const uint32_t ID = 0xf001;
			Header Header;
			int64_t Tick;
		};

		struct Frame
		{
			static const uint32_t ID = 0x0041;
			Header Header;
			int64_t BeginTick;
			int64_t EndTick;
			int64_t TickRate;
			uint32_t FrameNumber;
			uint32_t _reserved0_;
		};

		struct EnterScope
		{
			static const uint32_t ID = 0x00a0;
			Header Header;
			uint8_t ThreadId;
			uint8_t CpuId;
			uint8_t _pad0_[2];
			uint32_t Scope;
			int64_t Tick;
		};

		struct LeaveScope
		{
			static const uint32_t ID = 0x00b0;
			Header Header;
			uint8_t ThreadId;
			uint8_t CpuId;
			uint8_t _pad0_[2];
			uint32_t _reserved0_;
			int64_t Tick;
		};

		struct Packet
		{
			static const uint32_t ID = 0x4002;
			Header Header;
			uint32_t Flags;
			uint32_t _reserved0_;
		};

		static_assert(sizeof(Header)	%8 == 0, "Misaligned");
		static_assert(sizeof(Connect)	%8 == 0, "Misaligned");
		static_assert(sizeof(Frame)		%8 == 0, "Misaligned");
		static_assert(sizeof(EnterScope)%8 == 0, "Misaligned");
		static_assert(sizeof(LeaveScope)%8 == 0, "Misaligned");
		static_assert(sizeof(Packet)	%8 == 0, "Misaligned");
	}

} } }

namespace nemesis { namespace perf { namespace detail
{
	class Clock
	{
	public:
		static int64_t GetRate()
		{
			LARGE_INTEGER tick;
			QueryPerformanceFrequency( &tick );
			return tick.QuadPart;
		}

		static int64_t GetTick()
		{ 
			LARGE_INTEGER tick;
			QueryPerformanceCounter( &tick );
			return tick.QuadPart;
		}
	};

	class Socket
	{
	public:
		Socket() 
			: Instance(INVALID_SOCKET)
		{}
		~Socket() 
		{}

	public:
		Socket Accept()
		{
			Socket remote;
			remote.Instance = accept( Instance, nullptr, nullptr );
			return remote;
		}
		bool ListenTcp( int port )
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
		bool BindUdpNonBlocking( int port )
		{
			Close();
			Instance = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
			if (!IsOpen())
				return false;
			sockaddr_in addr;
			addr.sin_family		 = AF_INET;
			addr.sin_addr.s_addr = htonl( 0 );
			addr.sin_port		 = htons( port );
			if (bind( Instance, (sockaddr*)&addr, sizeof(addr) ) < 0)
				return false;
			DWORD non_blocking = 1;
			if (ioctlsocket( Instance, FIONBIO, &non_blocking ) < 0)
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
		bool Send( const void* data, uint32_t size, const sockaddr_in* addr )
		{
			if (!IsOpen())
				return false;
			const char* pos = (const char*)data;
			const char* end = pos + size;
			while ( pos < end )
			{
				int written = addr 
					? sendto( Instance, pos, (int)(end-pos), 0, (const sockaddr*)addr, (int)sizeof(*addr) ) 
					: send	( Instance, pos, (int)(end-pos), 0 );
				if (written > 0)
					pos += written;
				else
					return false;
			}
			return true;
		}
		bool Recv( void* data, uint32_t size, sockaddr_in* addr )
		{
			sockaddr_in address = {};
			int l = sizeof(sockaddr_in);
			int len = (int)size;
			int hr = recvfrom( Instance, (char*)data, len, 0, (sockaddr*)&address, &l );
			if (addr)
				*addr = address;
			return (hr == len);
		}
		static void StaticInitialize()
		{ WSAStartup( MAKEWORD(2,2), &WsaData ); }
		static void StaticShutdown()
		{ WSACleanup(); }
	private:
		SOCKET Instance;
		static WSADATA WsaData;
	};

	WSADATA Socket::WsaData = {};

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

		void Start( DWORD (__stdcall *proc)( void* arg ), void* arg )
		{
			if (Handle)
				return;
			Quit = 0;
			DWORD thread_id;
			Handle = CreateThread( nullptr, 0, proc, arg, 0, &thread_id );
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
		Sender()	
		{}

		~Sender()
		{ 
			Stop(); 
		}
	public:
		bool Start( int port )
		{
			if (Server.IsOpen())
				return false;
			if (!Server.ListenTcp( port ))
				return false;
			Worker.Start( Run, this );
			return true;
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
		static DWORD __stdcall Run( void* arg )
		{ 
			((Sender*)arg)->Accept(); 
			return 0;
		}
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
			{
				const packet::Connect item1 = { { packet::Connect::ID, sizeof(item1) }, Clock::GetTick() };
				const packet::Packet  item0 = { { packet::Packet::ID, sizeof(item0) + sizeof(item1) } };
				memcpy( buffer.Data				   , &item0, sizeof(item0) );
				memcpy( buffer.Data + sizeof(item0), &item1, sizeof(item1) );
				if (!Client.Send( buffer.Data, item0.Header.Size, nullptr ))
					return;
			}

			for ( ; Worker.Continue() ; )
			{
				Queue.Pop( buffer );
				if (!buffer.Size)
					continue;
				if (Client.Send( buffer.Data, buffer.Size, nullptr))
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

	class TlsRecorder
	{
	public:
		TlsRecorder( int index, Sender* sender )
			: Index( index )
			, Sender( sender )
		{
			ZeroMemory(&Data, sizeof(Data));
			Data.Size = sizeof(packet::Packet); // reserve space for packet header
		}

	public:
		void EnterScope( const char* tag, const char* func, const char* file, int line )
		{
			uint32_t scope_id = 0;

			const packet::EnterScope item = 
			{ { packet::EnterScope::ID, sizeof(item) }
			, Index
			, (uint8_t)GetCurrentProcessorNumber()
			, { 0, 0 }
			, scope_id
			, Clock::GetTick()
			};
			Record( &item.Header );
		}

		void LeaveScope()
		{
			uint32_t scope_id = 0;

			const packet::LeaveScope item = 
			{ { packet::LeaveScope::ID, sizeof(item) }
			, Index
			, (uint8_t)GetCurrentProcessorNumber()
			, { 0, 0 }
			, 0
			, Clock::GetTick()
			};
			Record( &item.Header );
		}

		void NextFrame()
		{
			Mutex.Enter();
			Flush();
			Mutex.Leave();
		}

		void Record( const packet::Header* data )
		{
			Mutex.Enter();
			if (Data.Size + data->Size > Data.NUM_BYTES)
				Flush();
			memcpy( Data.Data + Data.Size, data, data->Size );
			Data.Size += data->Size;
			Mutex.Leave();
		}

	private:
		void Flush()
		{
			if (Data.Size <= sizeof(packet::Packet))
				return;
			{
				const packet::Packet item = 
				{ { packet::Packet::ID, Data.Size }
				, 0, 0
				};
				memcpy( Data.Data, &item, sizeof(item) );
			}
			Sender->Send( Data );
			Data.Size = sizeof(packet::Packet);
		}

	private:
		Buffer Data;
		uint8_t Index;
		Sender* Sender;
		CriticalSection Mutex;
	};

	class Recorder
	{
	public:
		Recorder() 
			: Tls( TlsAlloc() )
			, Sender( nullptr )
			, BeginTick( Clock::GetTick() )
			, FrameNumber( 0 )
		{}

		~Recorder() 
		{ TlsFree( Tls ); }

	public:
		void Initialize( Sender* sender )
		{ Sender = sender; }

		void EnterScope( const char* tag, const char* func, const char* file, int line )
		{ return GetTlsRecorder()->EnterScope( tag, func, file, line ); }

		void LeaveScope()
		{ return GetTlsRecorder()->LeaveScope(); }

		void NextFrame()
		{
			Mutex.Enter();
			for ( int i = 0; i < TlsList.size(); ++i )
				TlsList[i]->NextFrame();
			Mutex.Leave();
			{
				const packet::Frame item = 
				{ { packet::Frame::ID, sizeof(item) }
				, BeginTick
				, Clock::GetTick()
				, Clock::GetRate()
				, FrameNumber++
				};
				GetTlsRecorder()->Record( &item.Header );
				BeginTick = item.EndTick;
			}
		}

	private:
		TlsRecorder* GetTlsRecorder()
		{
			TlsRecorder* tls_recorder = (TlsRecorder*)TlsGetValue( Tls );
			if (!tls_recorder)
			{
				{
					Mutex.Enter();
					tls_recorder = new TlsRecorder( TlsList.size(), Sender );
					TlsList.push_back(tls_recorder);
					Mutex.Leave();
				}
				TlsSetValue( Tls, tls_recorder );
			}
			return tls_recorder;
		}

	private:
		uint32_t Tls;
		Sender* Sender;
		int64_t BeginTick;
		uint32_t FrameNumber;
		CriticalSection Mutex;
		std::vector<TlsRecorder*> TlsList;
	};

	class Server
	{
	public:
		void Initialize()
		{
			Socket::StaticInitialize();
			Recorder.Initialize( &Sender );
		}

		void Shutdown()
		{
			Socket::StaticShutdown();
		}

		void StartSender( int port )
		{ 
			if (!Sender.Start( port ))
				return;
			Responder.BindUdpNonBlocking( port );
		}

		void StopSender()
		{ 
			Sender.Stop(); 
			Responder.Close();
		}

		void NextFrame()
		{
			Respond();
			Recorder.NextFrame();
		}

		void EnterScope( const char* tag, const char* func, const char* file, int line )
		{
			Recorder.EnterScope( tag, func, file, line );
		}

		void LeaveScope()
		{
			Recorder.LeaveScope();
		}

	private:
		void Respond()
		{
			ping::Header hdr = {};
			sockaddr_in from = {};
			if (!Responder.Recv( &hdr, sizeof(hdr), &from ))
				return;
			if (!hdr.IsValid())
				return;
			ping::Response ack = {};
			ack.Header.Init();
			Responder.Send( &ack, sizeof(ack), &from );
		}

	private:
		Sender   Sender;
		Recorder Recorder;
		Socket	 Responder;
	};

} } }

namespace nemesis
{
	static perf::detail::Server PerfServer;

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
