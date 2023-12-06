#pragma once

//======================================================================================
#include "Types.h"

//======================================================================================
//	Debug Types
//======================================================================================
namespace Nemesis
{
	struct DebugPopUp
	{
		enum Mode
		{ None
		, OkCancel
		, RetryCancelContinue
		};

		enum Result
		{ Ok
		, Retry
		, Cancel
		, Continue
		};
	};
}

//======================================================================================
//	Debug Api
//======================================================================================
namespace Nemesis
{
	void NE_API Debug_Print   ( cstr_t msg );
	void NE_API Debug_PrintLn ( cstr_t msg );
	void NE_API Debug_PrintF  ( cstr_t msg, ... );
	void NE_API Debug_PrintLnF( cstr_t msg, ... );

	DebugPopUp::Result NE_API Debug_PopUp( DebugPopUp::Mode mode, cstr_t caption, cstr_t msg );
}
