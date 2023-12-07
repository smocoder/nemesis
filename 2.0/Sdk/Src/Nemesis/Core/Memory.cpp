#include "stdafx.h"
#include <Nemesis/Core/Memory.h>
#include <Nemesis/Core/Assert.h>
#include <Nemesis/Core/Debug.h>
#include <memory.h>
#include <malloc.h>

//======================================================================================
//																			  Memory Api
//======================================================================================
namespace Nemesis
{
	ptr_t Mem_Zero( ptr_t dst, size_t size )
	{ 
		return memset( dst, 0, size ); 
	}

	ptr_t Mem_Set( ptr_t dst, int val, size_t size )
	{ 
		return memset( dst, val, size ); 
	}

	ptr_t Mem_Cpy( ptr_t dst, cptr_t src, size_t size )
	{ 
		return memcpy( dst, src, size ); 
	}

	ptr_t Mem_Cpy( ptr_t dst, size_t dst_size, cptr_t src, size_t src_size ) 
	{ 
		NeAssertOut( dst_size >= src_size, "Buffer Overflow! Destination buffer will be truncated." );
		const size_t truncated_size = NeMin( dst_size, src_size );
		return Mem_Cpy( dst, src, truncated_size ); 
	}

	ptr_t Mem_Mov( ptr_t dst, cptr_t src, size_t size )
	{ 
		return memmove( dst, src, size );
	}

	int	Mem_Cmp( cptr_t lhs, cptr_t rhs, size_t size )
	{ 
		return memcmp( lhs, rhs, size ); 
	}
}

//======================================================================================
//																		   Alloc Private
//======================================================================================
namespace Nemesis
{
	static ptr_t CrtAlloc_Realloc( Alloc_t alloc, ptr_t ptr, size_t size )
	{
		if (ptr == nullptr)
			return malloc( size );

		if (size == 0)
		{
			free( ptr );
			return nullptr;
		}

		return realloc( ptr, size );
	}

	static size_t CrtAlloc_SizeOf( Alloc_t alloc, ptr_t ptr )
	{
		if (!ptr)
			return 0;

	#if NE_PLATFORM_IS_WINDOWS
		return _msize( ptr );
	#else
		return malloc_usable_size( ptr );
	#endif
	}

}

//======================================================================================
namespace Nemesis
{
	namespace 
	{
		static Alloc_s CrtAlloc = { CrtAlloc_Realloc, CrtAlloc_SizeOf };
		static Alloc_t AllocHook = nullptr;
	}

	static Alloc_t Alloc_Select( Alloc_t alloc )
	{
		if (alloc)
			return alloc;
		if (AllocHook)
			return AllocHook;
		return Alloc_GetDefault();
	}

}

//======================================================================================
//																			   Alloc Api
//======================================================================================
namespace Nemesis
{
	Alloc_t Alloc_GetDefault()
	{
		return &CrtAlloc;
	}

	Alloc_t Mem_GetHook()
	{
		return AllocHook;
	}

	void Mem_SetHook( Alloc_t hook )
	{
		AllocHook = hook;
	}

	ptr_t Mem_Alloc( Alloc_t alloc, size_t size )
	{
		const Alloc_t inst = Alloc_Select( alloc );
		return inst->Realloc( inst, nullptr, size );
	}

	ptr_t Mem_Calloc( Alloc_t alloc, size_t size )
	{
		return Mem_Zero( Mem_Alloc( alloc, size ), size );
	}

	ptr_t Mem_Realloc( Alloc_t alloc, ptr_t ptr, size_t size )
	{
		const Alloc_t inst = Alloc_Select( alloc );
		return inst->Realloc( alloc, ptr, size );
	}

	void Mem_Free( Alloc_t alloc, ptr_t ptr )
	{
		const Alloc_t inst = Alloc_Select( alloc );
		inst->Realloc( inst, ptr, 0 );
	}

	size_t Mem_SizeOf( Alloc_t alloc, ptr_t ptr )
	{
		const Alloc_t inst = Alloc_Select( alloc );
		return inst->SizeOf( inst, ptr );
	}

	ptr_t Mem_Clone( Alloc_t alloc, cptr_t ptr, size_t size )
	{
		if (!ptr || !size)
			return nullptr;
		ptr_t clone = Mem_Alloc( alloc, size );
		Mem_Cpy( clone, ptr, size );
		return clone;
	}

}

//======================================================================================
//																	   Counted Allocator
//======================================================================================
namespace Nemesis
{
	static ptr_t NE_CALLBK CountedAlloc_Realloc( Alloc_t alloc, ptr_t ptr, size_t size )
	{
		CountedAlloc_t instance = (CountedAlloc_t)alloc;
		const size_t old_size = Mem_SizeOf	( instance->Parent, ptr );
		const ptr_t  new_ptr  = Mem_Realloc	( instance->Parent, ptr, size );
		const size_t new_size = Mem_SizeOf	( instance->Parent, new_ptr );
		instance->TotalBytes -= old_size;
		instance->TotalBytes += new_size;
		instance->PeakBytes  = NeMax( instance->PeakBytes, instance->TotalBytes );
		instance->TotalCalls ++;
		return new_ptr;
	}

	static size_t NE_CALLBK CountedAlloc_SizeOf( Alloc_t alloc, ptr_t ptr )
	{
		CountedAlloc_t instance = (CountedAlloc_t)alloc;
		return Mem_SizeOf( instance->Parent, ptr );
	}

	CountedAlloc_s CountedAlloc_Create( Alloc_t parent, cstr_t name )
	{
		return CountedAlloc_s { Alloc_s { CountedAlloc_Realloc, CountedAlloc_SizeOf }, parent, name };
	}

	void CountedAlloc_Dump( CountedAlloc_t alloc )
	{
		Debug_Print( "*************************\n" );
		Debug_PrintF( "*** Allocator: '%s'\n", alloc->Name );
		Debug_PrintF( "*** Num Calls:  %u\n", alloc->TotalCalls );
		Debug_PrintF( "*** Peak Bytes: %u\n", alloc->PeakBytes  );
		Debug_PrintF( "*** Leak Bytes: %u\n", alloc->TotalBytes );
	}

}