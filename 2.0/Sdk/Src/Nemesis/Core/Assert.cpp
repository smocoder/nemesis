#include "stdafx.h"
#include <Nemesis/Core/Assert.h>
#include <Nemesis/Core/Debug.h>
#include <Nemesis/Core/String.h>
#include <stdlib.h>	// exit

//======================================================================================
//																				 Private
//======================================================================================
namespace Nemesis
{
	static AssertAction::Kind NE_CALLBK Assert_DefaultHook( void*, const AssertInfo_s& info, cstr_t msg )
	{
		char buffer[1024] = "";
		Str_Fmt
			( buffer
			, "Assertion Failed!\n"
			  "\n"
			  "File: %s\n"
			  "Line: %u\n"
			  "\n"
			  "Expression: %s\n"
			  "\n"
			  "Comment: %s\n"
			  "\n"
			  "Press Retry to debug the application (JIT must be enabled)\n"
			  "\n"
			, info.File
			, info.Line
			, info.Expression
			, msg ? msg : "(not available)"
			);

		switch (Debug_PopUp( DebugPopUp::RetryCancelContinue, "Assertion Failure", buffer ))
		{
		case DebugPopUp::Retry:
			break;
		case DebugPopUp::Continue:
			return AssertAction::Continue;
		default:
			return AssertAction::Terminate;
		}
		return AssertAction::Break;
	}
}

//======================================================================================
namespace Nemesis
{
	namespace
	{
		static AssertHook_s AssertHook = { Assert_DefaultHook };
	}
}

//======================================================================================
namespace Nemesis
{
	static AssertAction::Kind Assert_FailureT( const AssertInfo_s& info, cstr_t msg )
	{
		return AssertHook.AssertFailed
			? AssertHook.AssertFailed( AssertHook.Context, info, msg )
			: AssertAction::Terminate;
	}

	static AssertAction::Kind Assert_FailureV( const AssertInfo_s& info, cstr_t msg, va_list args )
	{
		char text[ 1024 ] = "";
		Str_FmtV( text, msg, args );
		return Assert_FailureT( info, text );
	}
}

//======================================================================================
//																				  Public
//======================================================================================
namespace Nemesis
{
	AssertHook_s Assert_GetHook()
	{
		return AssertHook;
	}

	void Assert_SetHook()
	{
		AssertHook = { Assert_DefaultHook };
	}

	void Assert_SetHook( const AssertHook_s& hook )
	{
		AssertHook = hook;
	}

	AssertAction::Kind Assert_Failure( const AssertInfo_s& info, cstr_t msg, ... )
	{
		// call the hook
		va_list args;
		va_start( args, msg );
		const AssertAction::Kind action = Assert_FailureV( info, msg, args );
		va_end( args );

		// handle termination
		switch (action)
		{
		case AssertAction::Terminate:
			exit( 1 );
			break;
		default:
			break;
		}
		return action;
	}
}
