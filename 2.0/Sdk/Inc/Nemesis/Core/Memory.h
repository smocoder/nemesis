#pragma once

//======================================================================================
#include "Types.h"

//======================================================================================
//	Alloc Types
//======================================================================================
namespace Nemesis
{
	typedef struct Alloc_s *Alloc_t;

	struct Alloc_s
	{
		ptr_t	(NE_CALLBK *Realloc)( Alloc_t alloc, ptr_t ptr, size_t size );
		size_t	(NE_CALLBK *SizeOf) ( Alloc_t alloc, ptr_t ptr );
	};
}

//======================================================================================
//	Memory Api
//======================================================================================
namespace Nemesis
{
	ptr_t NE_API Mem_Zero( ptr_t dst, size_t size );
	ptr_t NE_API Mem_Set ( ptr_t dst, int val, size_t size );
	ptr_t NE_API Mem_Cpy ( ptr_t dst, cptr_t src, size_t size );
	ptr_t NE_API Mem_Cpy ( ptr_t dst, size_t dst_size, cptr_t src, size_t src_size );
	ptr_t NE_API Mem_Mov ( ptr_t dst, cptr_t src, size_t size );
	int	  NE_API Mem_Cmp ( cptr_t lhs, cptr_t rhs, size_t size );

	template < typename T, size_t S > 
	inline ptr_t Mem_Cpy( T (&dst)[S], cptr_t src, size_t src_size ) 
	{ return Mem_Cpy( dst, S * sizeof(T), src, src_size ); }

	template < typename T >
	inline void NeZero( T& v )
	{ Mem_Zero( &v, sizeof(v) ); }
}

//======================================================================================
//	Alloc Api
//======================================================================================
namespace Nemesis
{
	Alloc_t NE_API Alloc_GetDefault();

	Alloc_t	NE_API Mem_GetHook();
	void	NE_API Mem_SetHook( Alloc_t alloc );
	ptr_t	NE_API Mem_Alloc  ( Alloc_t alloc,			  size_t size );
	ptr_t	NE_API Mem_Calloc ( Alloc_t alloc,			  size_t size );
	ptr_t	NE_API Mem_Realloc( Alloc_t alloc, ptr_t ptr, size_t size );
	void	NE_API Mem_Free	  ( Alloc_t alloc, ptr_t ptr );
	size_t	NE_API Mem_SizeOf ( Alloc_t alloc, ptr_t ptr );
	ptr_t	NE_API Mem_Clone  ( Alloc_t alloc, cptr_t ptr, size_t size );
}

//======================================================================================
//	Typed Api
//======================================================================================
namespace Nemesis
{
	template < typename T >inline T* Mem_Alloc  ( Alloc_t alloc )					{ return (T*) Mem_Alloc  ( alloc	 ,	     sizeof(T) ); }
	template < typename T >inline T* Mem_Alloc  ( Alloc_t alloc, int num )			{ return (T*) Mem_Alloc  ( alloc	 , num * sizeof(T) ); }
	template < typename T >inline T* Mem_Calloc ( Alloc_t alloc )					{ return (T*) Mem_Calloc ( alloc	 ,	     sizeof(T) ); }
	template < typename T >inline T* Mem_Calloc ( Alloc_t alloc, int num )			{ return (T*) Mem_Calloc ( alloc	 , num * sizeof(T) ); }
	template < typename T >inline T* Arr_Realloc( Alloc_t alloc, T* ptr, int num )	{ return (T*) Mem_Realloc( alloc, ptr, num * sizeof(T) ); }

	template < typename T >	inline T* Arr_Zero( T* dst				, int count )	{ return (T*)Mem_Zero( dst		, count * sizeof(T) ); }
	template < typename T >	inline T* Arr_Cpy ( T* dst, const T* src, int count )	{ return (T*)Mem_Cpy ( dst, src , count * sizeof(T) ); }
	template < typename T >	inline T* Arr_Mov ( T* dst, const T* src, int count )	{ return (T*)Mem_Mov ( dst, src , count * sizeof(T) ); }
	template < typename T > inline T* Arr_Set ( T* dst, const T& src, int count )
	{
		for ( int i = 0; i < count; ++i )
			dst[i] = src;
		return dst;
	}
}

//======================================================================================
//	Counted Allocator
//======================================================================================
namespace Nemesis
{
	typedef struct CountedAlloc_s *CountedAlloc_t;

	struct CountedAlloc_s
	{
		Alloc_s Alloc;
		Alloc_t Parent;
		cstr_t Name;
		size_t TotalCalls;
		size_t TotalBytes;
		size_t PeakBytes;
	};

	CountedAlloc_s	NE_API CountedAlloc_Create( Alloc_t parent, cstr_t name );
	void			NE_API CountedAlloc_Dump( CountedAlloc_t alloc );

}
