#pragma once

//======================================================================================
#include <stdio.h>

//======================================================================================
namespace Nemesis
{
	void Debug_Print( cstr_t text )
	{
		printf( "%s", text );
	}

	void Debug_PrintLn( cstr_t text )
	{
		printf( "%s\n", text );
	}
}

//======================================================================================
namespace Nemesis
{
	DebugPopUp::Result Debug_PopUp( DebugPopUp::Mode mode, cstr_t caption, cstr_t msg )
	{
		Debug_PrintLn( msg );
		return DebugPopUp::Retry;
	}
}
