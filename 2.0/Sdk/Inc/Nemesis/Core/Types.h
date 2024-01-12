#pragma once

//======================================================================================
#include "Target.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

//======================================================================================
//	Global Templates
//======================================================================================
template < typename T >
inline T NeMin( const T& a, const T& b )
{ return a < b ? a : b; }

template < typename T >
inline T NeMax( const T& a, const T& b )
{ return a > b ? a : b; }

template < typename T >
inline T NeClamp( const T& value, const T& minimum, const T& maximum )
{ return NeMin( NeMax( value, minimum ), maximum ); }

template < typename T >
inline void NeSwap( T& a, T& b )
{ 
	const T temp = b;
	b = a;
	a = temp;
}

inline uint32_t Mem_Align_Pow2( uint32_t pos, uint32_t align )
{ return (pos + (align-1)) & ~(align-1); }

inline uint32_t Mem_Align_16( uint32_t pos )
{ return (pos + (16-1)) & ~(16-1); }


//======================================================================================
//	Global Macros
//======================================================================================
#define NeUnused(x)			((void)(x))
#define NeCountOf(a)		(sizeof(a)/sizeof(a[0]))
#define NeUnique(prefix)	NeConcat(prefix, __LINE__)

#define NeHasFlag(style, flag)		(((style) & (flag)) == (flag))
#define NeSetFlag(style, flag, on)	((style) = (on) ? ((style)|(flag)) : ((style)&(~(flag))))

#define NeMakeFourCc( a, b, c, d )  ( ((d) << 24) | ((c) << 16) | ((b) << 8) | (a) )

//======================================================================================
#if (NE_COMPILER == NE_COMPILER_MSVC)
#	define NeWeak __declspec(selectany) 
#else
#	define NeWeak __attribute__((weak))
#endif

//======================================================================================
//  Fundamental Types
//======================================================================================
namespace Nemesis
{
	typedef void*			ptr_t;
	typedef const void*		cptr_t;

	typedef char*			str_t;
	typedef const char*		cstr_t;

	typedef wchar_t*		wstr_t;
	typedef const wchar_t*	wcstr_t;

	typedef int				Result_t;
}

//======================================================================================
//	Fundamental Functions
//======================================================================================
namespace Nemesis
{
	//----------------------------------------------------------------------------------

	inline bool Size_IsPow2( size_t size )
	{
		return ((size != 0) && (size & (size-1)) == 0);
	}

	inline size_t Size_Align( size_t size, size_t align )
	{
		return ((size + (align - 1)) / align) * align;
	}

	inline size_t Size_Align_Pow2( size_t size, size_t align )
	{
		return (size + (align - 1)) & (~(align - 1));
	}

	inline bool Size_IsAligned_Pow2( size_t size, size_t align )
	{
		return (size & (align-1)) == 0;
	}

	//----------------------------------------------------------------------------------

	inline ptrdiff_t PtrDiff_Align_Pow2( ptrdiff_t pos, size_t align )
	{
		return (pos + (align-1)) & ~(align-1);
	}

	//----------------------------------------------------------------------------------

	inline uint32_t Mem_Align_Pow2( uint32_t pos, uint32_t align  )
	{
		return (pos + (align-1)) & ~(align-1);
	}

	inline uint32_t Mem_Align_16( uint32_t pos )
	{
		return (pos + (16-1)) & ~(16-1);
	}

	//----------------------------------------------------------------------------------

	inline ptrdiff_t Ptr_Tell( cptr_t src, cptr_t dst )
	{
		return ((uint8_t*)dst) - ((uint8_t*)src);
	}

	inline ptr_t Ptr_Seek( ptr_t ptr, ptrdiff_t offset )
	{
		return ((uint8_t*)ptr) + offset;
	}

	inline cptr_t Ptr_Seek( cptr_t ptr, ptrdiff_t offset )
	{
		return ((const uint8_t*)ptr) + offset;
	}

	inline ptr_t Ptr_Align_Pow2( ptr_t pos, size_t align )
	{
		return (ptr_t)PtrDiff_Align_Pow2( (ptrdiff_t)pos, align );
	}

	inline cptr_t Ptr_Align_Pow2( cptr_t pos, size_t align )
	{
		return (ptr_t)PtrDiff_Align_Pow2( (ptrdiff_t)pos, align );
	}

	inline bool Ptr_IsAligned_Pow2( cptr_t pos, size_t align )
	{
		return Size_IsAligned_Pow2( (size_t)pos, align );
	}

}

//======================================================================================
//	Result Values
//======================================================================================
#define NE_OK					0
#define NE_ERROR				0x8000000
#define NE_ERR_CORE				0x0010000
#define NE_ERR_INVALID_ARG		(NE_ERROR | NE_ERR_CORE | 0x1)
#define NE_ERR_INVALID_CALL		(NE_ERROR | NE_ERR_CORE | 0x2)
#define NE_ERR_OUT_OF_MEMORY	(NE_ERROR | NE_ERR_CORE | 0x3)
#define NE_ERR_NOT_SUPPORTED	(NE_ERROR | NE_ERR_CORE | 0x4)
#define NE_ERR_NOT_FOUND		(NE_ERROR | NE_ERR_CORE | 0x5)

#define NeFailed( hr )		((hr & NE_ERROR) == NE_ERROR)
#define NeSucceeded( hr )	((hr & NE_ERROR) == 0)

//======================================================================================
//	Abbreviations
//======================================================================================
namespace Ne = Nemesis;
