#include "stdafx.h"
#include <Nemesis/Core/Debug.h>
#include <Nemesis/Core/String.h>

//======================================================================================
#if ((NE_PLATFORM == NE_PLATFORM_WIN32) || (NE_PLATFORM == NE_PLATFORM_WIN64))
#	include "Debug_Windows.h"
#elif (NE_PLATFORM == NE_PLATFORM_DURANGO)
#	include "Debug_Null.h"
#elif (NE_PLATFORM == NE_PLATFORM_SCARLETT)
#	include "Debug_Null.h"
#elif (NE_PLATFORM == NE_PLATFORM_PS4)
#	include "Debug_Null.h"
#elif (NE_PLATFORM == NE_PLATFORM_PS5)
#	include "Debug_Null.h"
#else
#	error Unrecognized platform...
#endif

//======================================================================================
namespace Nemesis
{
	void Debug_PrintF( cstr_t msg, ... )
	{
		va_list args;
		va_start( args, msg );
		char text[1024] = "";
		Str_FmtV( text, msg, args );
		Debug_Print( text );
		va_end( args );
	}

	void Debug_PrintLnF( cstr_t msg, ... )
	{
		va_list args;
		va_start( args, msg );
		char text[1024] = "";
		Str_FmtV( text, msg, args );
		Debug_PrintLn( text );
		va_end( args );
	}
}
