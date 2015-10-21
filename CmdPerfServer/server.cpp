#include "server.h"
#include <SDKDDKVer.h>
#include <Windows.h>
#include <stdint.h>
#include <atomic>
#include <vector>
#pragma comment (lib, "ws2_32.lib")

#define NeMakeFourCc( a, b, c, d ) ( ((a) << 24) | ((b) << 16) | ((c) << 8) | (d) )

namespace nemesis
{
	struct IntrinsicComparer
	{
		template < typename T >
		static int Compare( const T& lhs, const T& rhs )
		{
			if (lhs > rhs)
				return 1;
			if (lhs < rhs)
				return -1;
			return 0;
		}

		template < typename T >
		static bool Equals( const T& lhs, const T& rhs )
		{ return lhs == rhs; }
	};

	struct BitwiseComparer
	{
		template < typename T >
		static int Compare( const T& lhs, const T& rhs )
		{ return memcmp( &lhs, &rhs, sizeof(T) ); }

		template < typename T >
		static bool Equals( const T& lhs, const T& rhs )
		{ return 0 == Compare( lhs, rhs ); }
	};

	struct BinaryFindResult
	{
		uint32_t Index;
		bool	 Found;

		BinaryFindResult( uint32_t index, bool found )
			: Index(index)
			, Found(found)
		{}
	};

	template < typename C, typename T, typename K >
	BinaryFindResult BinaryFind( const T& a, const K& v )
	{
		int cmp;
		int32_t pivot;
		int32_t start = 0;
		int32_t end = ((int32_t)a.size())-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			cmp = C::Compare(a[pivot], v);
			if ( 0 == cmp )
				return BinaryFindResult( pivot, true );
			if ( cmp > 0 )
				end = pivot-1;
			else
				start = pivot+1;
		}
		return BinaryFindResult( start, false );
	}

	template < typename T, typename K >
	BinaryFindResult BinaryFind( const T& a, const K& v )
	{ return BinaryFind<IntrinsicComparer>( a, v ); }
}

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

		struct NameList
		{
			static const uint32_t ID = 0x0102;
			#pragma pack(push)
			#pragma pack(1)
			struct Item
			{
				uint64_t Name;
				uint16_t Length;
			};
			#pragma pack(pop)
			Header Header;
			uint32_t Count;
			uint32_t _pad0_;
		};

		struct LocationList
		{
			static const uint32_t ID = 0x0103;
			struct Item
			{
				uint64_t Name;
				uint64_t Func;
				uint64_t File;
				uint32_t Line;
				uint32_t Id;
			};
			Header Header;
			uint32_t Count;
			uint8_t ThreadId;
			uint8_t _pad0_[3];
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
		enum { NUM_BYTES = 64 * 1024 };
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
			: ClientId(0)
		{}

		~Sender()
		{ 
			Stop();
		}
	public:
		uint32_t GetClientId() const
		{
			return ClientId;
		}

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
					++ClientId;
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
		std::atomic<uint32_t> ClientId;
	};

	struct Location
	{
		const char* Name;
		const char* Func;
		const char* File;
		uint32_t Line;
	};

	class NameMap
	{
	public:
		void Register( const char* name )
		{
			const BinaryFindResult find = BinaryFind( Set, name );
			if (find.Found)
				return;
			Set.insert( Set.begin() + find.Index, name );
			Name.push_back( name );
			Length.push_back( (uint16_t)(1+strlen(name)) );
		}

	public:
		std::vector<const char*> Set;
		std::vector<const char*> Name;
		std::vector<uint16_t> Length;
	};

	class LocationMap
	{
	public:
		struct Pair
		{
			Location Scope;
			uint32_t Index;
		};
		struct PairComparer
		{
			static int Compare( const Pair& lhs, const Location& rhs )
			{ return BitwiseComparer::Compare(lhs.Scope, rhs); }
		};

		uint32_t Register( const Location& scope )
		{
			const BinaryFindResult find = BinaryFind<PairComparer>( Set, scope );
			if (find.Found)
				return Set[find.Index].Index;
			const Pair pair = { scope, Scope.size() };
			Set.insert( Set.begin() + find.Index, pair );
			Scope.push_back( scope );
			return pair.Index;
		}

	public:
		std::vector<Pair> Set;
		std::vector<Location> Scope;
	};

	class TlsRecorder
	{
	public:
		TlsRecorder( int index, Sender* sender )
			: Index( index )
			, Sender( sender )
			, FirstName( 0 )
			, FirstScope( 0 )
			, ClientId( 0 )
		{
			ZeroMemory(&Data, sizeof(Data));
			ZeroMemory(&Meta, sizeof(Meta));
			Data.Size = sizeof(packet::Packet);
			Meta.Size = sizeof(packet::Packet);
		}

	public:
		void EnterScope( const char* tag, const char* func, const char* file, int line )
		{
			Mutex.Enter();
			const Location scope = 
			{ tag
			, func
			, file
			, line 
			};
			const packet::EnterScope item = 
			{ { packet::EnterScope::ID, sizeof(item) }
			, Index
			, (uint8_t)GetCurrentProcessorNumber()
			, { 0, 0 }
			, Register( scope )
			, Clock::GetTick()
			};
			RecordNoLock( &item.Header );
			Mutex.Leave();
		}

		void LeaveScope()
		{
			Mutex.Enter();
			const packet::LeaveScope item = 
			{ { packet::LeaveScope::ID, sizeof(item) }
			, Index
			, (uint8_t)GetCurrentProcessorNumber()
			, { 0, 0 }
			, 0
			, Clock::GetTick()
			};
			RecordNoLock( &item.Header );
			Mutex.Leave();
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
			RecordNoLock( data );
			Mutex.Leave();
		}

	private:
		void Flush()
		{
			WriteMeta();
			Flush( Meta );
			Flush( Data );
		}
		void WriteMeta()
		{
			const uint32_t client_id = Sender->GetClientId();
			if (client_id != ClientId)
			{
				FirstName = 0;
				FirstScope = 0;
				ClientId = client_id;
				Meta.Size = sizeof(packet::Packet);
			}
			WriteNames();
			WriteScopes();
		}
		void WriteNames()
		{
			const uint32_t num_names = Name.Name.size() - FirstName;
			for ( uint32_t i = 0; i < num_names; ++i )
			{
				const uint32_t size = sizeof( packet::NameList ) + sizeof( packet::NameList::Item ) + Name.Length[i];
				if ( Meta.Size + size > Buffer::NUM_BYTES )
					Flush( Meta );

				const packet::NameList::Item item = 
				{ (uint64_t)Name.Name[i]
				, (uint16_t)Name.Length[i]
				};
				const packet::NameList list = 
				{ { packet::NameList::ID, size } 
				, 1
				, 0
				};
				memcpy( Meta.Data + Meta.Size, &list, sizeof(list) );			Meta.Size += sizeof(list);
				memcpy( Meta.Data + Meta.Size, &item, sizeof(item) );			Meta.Size += sizeof(item);
				memcpy( Meta.Data + Meta.Size, Name.Name[i], Name.Length[i] );	Meta.Size += Name.Length[i];
			}
			FirstName = Name.Name.size();
		}
		void WriteScopes()
		{
			const uint32_t num_scopes = Scope.Scope.size() - FirstScope;
			for ( uint32_t i = 0; i < num_scopes; ++i )
			{
				const uint32_t size = sizeof( packet::LocationList ) + sizeof( packet::LocationList::Item );
				if ( Meta.Size + size > Buffer::NUM_BYTES )
					Flush( Meta );

				const Location& scope = Scope.Scope[i];
				const packet::LocationList::Item item = 
				{ (uint64_t)scope.Name
				, (uint64_t)scope.Func
				, (uint64_t)scope.File
				, (uint32_t)scope.Line
				, i
				};
				const packet::LocationList list = 
				{ { packet::LocationList::ID, size } 
				, 1
				, Index
				, 0
				};
				memcpy( Meta.Data + Meta.Size, &list, sizeof(list) ); Meta.Size += sizeof(list);
				memcpy( Meta.Data + Meta.Size, &item, sizeof(item) ); Meta.Size += sizeof(item);
			}
			FirstScope = Scope.Scope.size();
		}
		void Flush( Buffer& data )
		{
			if (data.Size <= sizeof(packet::Packet))
				return;
			{
				const packet::Packet item = 
				{ { packet::Packet::ID, data.Size }
				, 0, 0
				};
				memcpy( data.Data, &item, sizeof(item) );
			}
			Sender->Send( data );
			data.Size = sizeof(packet::Packet);
		}
		void RecordNoLock( const packet::Header* header )
		{
			RecordNoLock( Data, header, header->Size );
		}
		void RecordNoLock( Buffer& data, const void* p, uint32_t s )
		{
			if ((data.Size + s) > Buffer::NUM_BYTES)
				Flush( data );
			memcpy( data.Data + data.Size, p, s );
			data.Size += s;
		}

		uint32_t Register( const Location& scope )
		{
			Name.Register( scope.Name );
			Name.Register( scope.Func );
			Name.Register( scope.File );
			return Scope.Register( scope );
		}

	private:
		uint8_t Index;
		Sender* Sender;
		Buffer Data;
		Buffer Meta;
		NameMap Name;
		LocationMap Scope;
		uint32_t FirstName;
		uint32_t FirstScope;
		uint32_t ClientId;
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
			{
				Mutex.Enter();
				{
					for ( uint32_t i = 0; i < Item.size(); ++i )
						Item[i]->NextFrame();
				}
				Mutex.Leave();
			}
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
					tls_recorder = new TlsRecorder( Item.size(), Sender );
					Item.push_back(tls_recorder);
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
		std::vector<TlsRecorder*> Item;
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
