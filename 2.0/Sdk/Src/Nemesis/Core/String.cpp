#include "stdafx.h"
#include <Nemesis/Core/String.h>
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

