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
		ptr_t  (NE_CALLBK *Realloc)( Alloc_t alloc, ptr_t ptr, size_t size );
		size_t (NE_CALLBK *SizeOf) ( Alloc_t alloc, ptr_t ptr );
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

	template < typename T >inline T* Mem_Alloc ( Alloc_t alloc )			{ return (T*) Mem_Alloc ( alloc,	   sizeof(T) ); }
	template < typename T >inline T* Mem_Alloc ( Alloc_t alloc, int num )	{ return (T*) Mem_Alloc ( alloc, num * sizeof(T) ); }
	template < typename T >inline T* Mem_Calloc( Alloc_t alloc )			{ return (T*) Mem_Calloc( alloc,	   sizeof(T) ); }
	template < typename T >inline T* Mem_Calloc( Alloc_t alloc, int num )	{ return (T*) Mem_Calloc( alloc, num * sizeof(T) ); }
}
