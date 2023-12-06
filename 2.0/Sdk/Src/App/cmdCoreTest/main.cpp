#include "stdafx.h"
#include <malloc.h>

//======================================================================================
using namespace Nemesis;

//======================================================================================
static ptr_t MyAlloc_Realloc( ptr_t context, ptr_t ptr, size_t size )
{
	ptr_t new_ptr = realloc( ptr, size );
	Debug_PrintLnF("%s(%d): Realloc: 0x%p, %u -> 0x%p", __FILE__, __LINE__, ptr, size, new_ptr);
	return new_ptr;
}

static size_t MyAlloc_SizeOf( ptr_t context, ptr_t ptr )
{
	return _msize( ptr );
}

static Alloc_s MyAlloc = { MyAlloc_Realloc, MyAlloc_SizeOf, nullptr };

//======================================================================================
static AssertAction::Kind MyAssertHook( ptr_t context, const AssertInfo_s& info, cstr_t msg )
{
	Debug_PrintLnF("%s(%d): Assertion Failed: %s", info.File, info.Line, info.Expression);
	return AssertAction::Continue;
}

//======================================================================================
int main( int argc, const char** argv )
{
	Assert_SetHook( AssertHook_s { MyAssertHook, nullptr } );
	NeAssert(0 == 1);

	Alloc_t alloc = nullptr;
	{
		int* p0 = (int*)Mem_Alloc( alloc, 10 * sizeof(int) );
		int* p1 = (int*)Mem_Calloc( alloc, 10 * sizeof(int) );
		int* p2 = Mem_Alloc<int>( alloc, 10 );
		int* p3 = Mem_Calloc<int>( alloc, 10 );
		Mem_Free( alloc, p3 );
		Mem_Free( alloc, p2 );
		Mem_Free( alloc, p1 );
		Mem_Free( alloc, p0 );
	}

	Mem_SetHook( &MyAlloc );

	{
		int* p0 = (int*)Mem_Alloc( alloc, 10 * sizeof(int) );
		int* p1 = (int*)Mem_Calloc( alloc, 10 * sizeof(int) );
		int* p2 = Mem_Alloc<int>( alloc, 10 );
		int* p3 = Mem_Calloc<int>( alloc, 10 );
		Mem_Free( alloc, p3 );
		Mem_Free( alloc, p2 );
		Mem_Free( alloc, p1 );
		Mem_Free( alloc, p0 );
	}

	Mem_SetHook( nullptr );

	return 0;
}
