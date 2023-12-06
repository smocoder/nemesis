#pragma once

//======================================================================================
#include "Target.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

//======================================================================================
//                                      templates
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

//======================================================================================
//                                      macros
//======================================================================================
#define NeUnused(x)			((void)(x))
#define NeCountOf(a)		(sizeof(a)/sizeof(a[0]))
#define NeUnique(prefix)	NeConcat(prefix, __LINE__)

#define NeHasFlag(style, flag)		(((style) & (flag)) == (flag))
#define NeSetFlag(style, flag, on)	((style) = (on) ? ((style)|(flag)) : ((style)&(~(flag))))

#define NeMakeFourCc( a, b, c, d )  ( ((d) << 24) | ((c) << 16) | ((b) << 8) | (a) )

#define NE_NO_COPY(...)\
	private:\
		__VA_ARGS__( const __VA_ARGS__& );\
		__VA_ARGS__& operator = ( const __VA_ARGS__& )

#define NE_DECL_INTERFACE(x)\
	protected:\
		virtual ~x() {}\
	public:

//======================================================================================
#if (NE_COMPILER == NE_COMPILER_MSVC)
#	define NeWeak __declspec(selectany) 
#else
#	define NeWeak __attribute__((weak))
#endif

//======================================================================================
//                                      types
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
//                                      results
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
namespace Ne = Nemesis;
