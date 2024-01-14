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

//======================================================================================
namespace Nemesis
{
	int Utf8_Len( cstr_t text )
	{
		int n;
		int len = 0;
		uint32_t cp;
		cstr_t pos = text;
		for ( ; pos; )
		{
			n = Utf8_Decode( pos, nullptr, cp );
			if (!n)
				break;
			len += n;
			pos += n;
		}
		return len;
	}

	int Utf8_Len( cstr_t start, cstr_t end )
	{
		int n;
		int len = 0;
		uint32_t cp;
		cstr_t pos = start;
		for ( ; pos && (pos < end); )
		{
			n = Utf8_Decode( pos, end, cp );
			if (!n)
				break;
			len += n;
			pos += n;
		}
		return len;
	}

	int Utf8_Decode( cstr_t start, cstr_t end, uint32_t& codepoint )
	{
		if (!start || !start[0] || (start == end))
		{
			codepoint = 0;
			return 0;
		}

		//
		//	bytes	cp bits		first cp	last cp		byte 1		byte 2		 byte 3		byte 4
		//	1		 7			U+0000		U+007F		0xxxxxxx			
		//	2		11			U+0080		U+07FF		110xxxxx	10xxxxxx		
		//	3		16			U+0800		U+FFFF		1110xxxx	10xxxxxx	10xxxxxx	
		//	4		21			U+10000		U+10FFFF	11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
		//
		uint32_t cp = (uint32_t)-1;
		const uint8_t* pos = (const uint8_t*)start;
		if (!(*pos & 0x80))
		{
			cp = (uint32_t)(*pos++);
			codepoint = cp;
			return 1;
		}
		if ((*pos & 0xe0) == 0xc0)
		{
			codepoint = 0xfffd; // will be invalid but not end of posing
			if (end && end - (cstr_t)pos < 2) 
				return 1;
			if (*pos < 0xc2) 
				return 2;
			cp = (uint32_t)((*pos++ & 0x1f) << 6);
			if ((*pos & 0xc0) != 0x80) 
				return 2;
			cp += (*pos++ & 0x3f);
			codepoint = cp;
			return 2;
		}
		if ((*pos & 0xf0) == 0xe0)
		{
			codepoint = 0xfffd; // will be invalid but not end of posing
			if (end && end - (cstr_t)pos < 3) 
				return 1;
			if (*pos == 0xe0 && (pos[1] < 0xa0 || pos[1] > 0xbf)) 
				return 3;
			if (*pos == 0xed && pos[1] > 0x9f) 
				return 3; // pos[1] < 0x80 is checked below
			cp = (uint32_t)((*pos++ & 0x0f) << 12);
			if ((*pos & 0xc0) != 0x80) 
				return 3;
			cp += (uint32_t)((*pos++ & 0x3f) << 6);
			if ((*pos & 0xc0) != 0x80) 
				return 3;
			cp += (*pos++ & 0x3f);
			codepoint = cp;
			return 3;
		}
		if ((*pos & 0xf8) == 0xf0)
		{
			codepoint = 0xfffd; // will be invalid but not end of posing
			if (end && end - (cstr_t)pos < 4) 
				return 1;
			if (*pos > 0xf4) 
				return 4;
			if (*pos == 0xf0 && (pos[1] < 0x90 || pos[1] > 0xbf)) 
				return 4;
			if (*pos == 0xf4 && pos[1] > 0x8f) 
				return 4; // pos[1] < 0x80 is checked below
			cp = (uint32_t)((*pos++ & 0x07) << 18);
			if ((*pos & 0xc0) != 0x80) 
				return 4;
			cp += (uint32_t)((*pos++ & 0x3f) << 12);
			if ((*pos & 0xc0) != 0x80) 
				return 4;
			cp += (uint32_t)((*pos++ & 0x3f) << 6);
			if ((*pos & 0xc0) != 0x80) 
				return 4;
			cp += (*pos++ & 0x3f);
			// utf-8 encodings of values used in surrogate pairs are invalid
			if ((cp & 0xFFFFF800) == 0xD800) 
				return 4;
			codepoint = cp;
			return 4;
		}
		codepoint = 0;
		return 0;	
	}

}