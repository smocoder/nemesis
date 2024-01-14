#include "stdafx.h"
#include <Nemesis/Core/String.h>
#include <Nemesis/Core/Assert.h>
#include <stdio.h>	// vsprintf_s

//======================================================================================
namespace Nemesis
{
	int Str_Len( cstr_t t )
	{
		return (int)(t ? strlen( t ) : 0);
	}

	int Str_Cmp( cstr_t lhs, cstr_t rhs )
	{
		return strcmp( lhs, rhs );
	}

	int Str_Cmp( cstr_t lhs, cstr_t rhs, size_t len )
	{
		return strncmp( lhs, rhs, len );
	}

	int Str_CmpNoCase( cstr_t lhs, cstr_t rhs )
	{
	#if NE_PLATFORM_IS_WINDOWS
		return _stricmp( lhs, rhs );
	#else
		return strcasecmp( lhs, rhs );
	#endif
	}
	
	str_t Str_Chr( str_t s, int ch )
	{
		return strchr( s, ch );
	}

	cstr_t Str_Chr( cstr_t s, int ch )
	{
		return strchr( s, ch );
	}

	cstr_t Str_ChrR( cstr_t s, int ch )
	{
		return strrchr( s, ch );
	}

	str_t Str_Str( str_t text, cstr_t find )
	{
		return strstr( text, find );
	}

	cstr_t Str_Str( cstr_t text, cstr_t find )
	{
		return strstr( text, find );
	}

	str_t Str_Cpy( str_t dst, size_t dst_size, cstr_t src )
	{ 
		const size_t src_size = src ? 1+strlen(src) : 0; // includes zero-termination
		NeAssertOut( dst_size >= src_size, "Buffer Overflow! Destination buffer will be truncated." );

		// handle the common case 
		if (src_size && (src_size <= dst_size))
		{
			memcpy( dst, src, src_size );
			return dst;
		}

		// handle edge cases
		if (!dst || (dst_size == 0))
			return dst;	// nothing to write to

		if (!src || (src_size == 0))
		{
			dst[0] = 0; // nothing to read from
			return dst;
		}

		// handle the truncting case
		const size_t full_size = NeMin( dst_size, src_size );	// w/  zero-termination
		const size_t text_size = full_size-1;					// w/o zero-termination
		memcpy( dst, src, text_size );
		dst[full_size] = 0;
		return dst;
	}

	str_t Str_Cat( str_t dst, size_t dst_size, cstr_t src )
	{
		const size_t dst_len = strlen(dst);
		const size_t src_len = strlen(src);
		const size_t cat_size = dst_len + src_len + 1;
		const size_t truncated_size = NeMin( cat_size, dst_size );
		NeAssertOut( cat_size <= truncated_size, "Buffer Overflow! Destination buffer will be truncated." );
		const size_t copy_len = truncated_size - dst_len - 1;
		memcpy( dst + dst_len, src, copy_len ); 
		dst[truncated_size-1] = 0;
		return dst;
	}

	int Str_FmtV( str_t out, size_t size, cstr_t fmt, va_list args )
	{
		return vsprintf_s( out, size, fmt, args );
	}

	int Str_Fmt( str_t out, size_t size, cstr_t fmt, ... )
	{
		va_list args;
		va_start( args, fmt );
		const int hr = Str_FmtV( out, size, fmt, args );
		va_end( args );
		return hr;
	}

	int Str_FmtCntV( cstr_t fmt, va_list args )
	{
		va_list copy;
		va_copy(copy, args);
		const int c = vsnprintf(nullptr, 0, fmt, copy);
		va_end(copy);
		return c;
	}
}

