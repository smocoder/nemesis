#pragma once

//======================================================================================
#include <stdio.h>

//======================================================================================
namespace Nemesis
{
	void Debug_Print( cstr_t msg )
	{
		OutputDebugStringA( msg );
	}

	void Debug_PrintLn( cstr_t msg )
	{
		char text[1024];
		sprintf_s( text, "%s\n", msg );
		Debug_Print( text );
	}
}

//======================================================================================
namespace Nemesis
{
	DebugPopUp::Result Debug_PopUp( DebugPopUp::Mode mode, cstr_t caption, cstr_t msg )
	{
		UINT type;
		switch ( mode )
		{
		case DebugPopUp::None:
			type = MB_OK;
			break;
		case DebugPopUp::OkCancel:
			type = MB_OKCANCEL;
			break;
		default:
			type = MB_CANCELTRYCONTINUE;
			break;
		}
		switch (::MessageBoxA( nullptr, msg, caption, MB_ICONERROR | type ))
		{
		case IDOK:
			return DebugPopUp::Ok;
		case IDTRYAGAIN:
			break;
		case IDCONTINUE:
			return DebugPopUp::Continue;
		case IDCANCEL:
			return DebugPopUp::Cancel;
		default:
			break;
		}
		return DebugPopUp::Retry;
	}
}
