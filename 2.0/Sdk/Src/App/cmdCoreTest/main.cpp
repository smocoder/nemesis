#include "stdafx.h"

//======================================================================================
using namespace Nemesis;

//======================================================================================
static AssertAction::Kind MyAssertHook( ptr_t context, const AssertInfo_s& info, cstr_t msg )
{
	Debug_PrintLnF("%s(%d): Assertion Failed: %s", info.File, info.Line, info.Expression);
	return AssertAction::Break;
}

//======================================================================================
int main( int argc, const char** argv )
{
	Assert_SetHook( AssertHook_s { MyAssertHook, nullptr } );
	NeAssert(0 == 1);
	return 0;
}
