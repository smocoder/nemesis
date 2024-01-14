#pragma once

//======================================================================================
#include "Types.h"

//======================================================================================
namespace Nemesis
{
	int		NE_API Str_Len		( cstr_t lhs );
	int		NE_API Str_Cmp		( cstr_t lhs, cstr_t rhs );
	int		NE_API Str_Cmp		( cstr_t lhs, cstr_t rhs, size_t len );
	int		NE_API Str_CmpNoCase( cstr_t lhs, cstr_t rhs );
	str_t	NE_API Str_Chr		( str_t s, int ch );
	cstr_t	NE_API Str_Chr		( cstr_t s, int ch );
	cstr_t	NE_API Str_ChrR		( cstr_t s, int ch );
	str_t	NE_API Str_Str		( str_t text, cstr_t find );
	cstr_t	NE_API Str_Str		( cstr_t text, cstr_t find );
	str_t	NE_API Str_Cpy		( str_t dst, size_t dst_size, cstr_t src );
	str_t	NE_API Str_Cat		( str_t dst, size_t dst_size, cstr_t src );
	int		NE_API Str_FmtV		( str_t out, size_t size, cstr_t fmt, va_list args );
	int		NE_API Str_Fmt		( str_t out, size_t size, cstr_t fmt, ... );
	int		NE_API Str_FmtCntV	( cstr_t fmt, va_list args );

	inline bool Str_Eq		( cstr_t lhs, cstr_t rhs )				{ return Str_Cmp( lhs, rhs )	   == 0; }
	inline bool Str_EqN		( cstr_t lhs, cstr_t rhs, size_t len )	{ return Str_Cmp( lhs, rhs, len )  == 0; }
	inline bool Str_EqNoCase( cstr_t lhs, cstr_t rhs )				{ return Str_CmpNoCase( lhs, rhs ) == 0; }

	template< size_t S > inline str_t Str_Cpy	( char (&dst)[S], cstr_t src )					{ return Str_Cpy( dst, S, src ); }
	template< size_t S > inline str_t Str_Cat	( char (&dst)[S], cstr_t src )					{ return Str_Cat( dst, S, src ); }
	template< size_t S > inline int	  Str_FmtV	( char (&out)[S], cstr_t fmt, va_list args )	{ return Str_FmtV( out, S, fmt, args ); }
	template< size_t S > inline int	  Str_Fmt	( char (&out)[S], cstr_t fmt, ... )			
	{ 
		va_list args;
		va_start( args, fmt );
		const int n = Str_FmtV( out, fmt, args );
		va_end( args );
		return n;
	}
}

//======================================================================================
namespace Nemesis
{
	int NE_API Utf8_Len		( cstr_t text );
	int NE_API Utf8_Len		( cstr_t start, cstr_t end );
	int NE_API Utf8_Decode	( cstr_t start, cstr_t end, uint32_t& codepoint );
}
