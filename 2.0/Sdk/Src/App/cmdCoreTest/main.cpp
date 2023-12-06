#include "stdafx.h"
#include <malloc.h>

//======================================================================================
using namespace Nemesis;

//======================================================================================
static ptr_t TraceAlloc_Realloc( Alloc_t alloc, ptr_t ptr, size_t size )
{
	ptr_t new_ptr = realloc( ptr, size );
	Debug_PrintLnF("%s(%d): Realloc: 0x%p, %u -> 0x%p", __FILE__, __LINE__, ptr, size, new_ptr);
	return new_ptr;
}

static size_t TraceAlloc_SizeOf( Alloc_t alloc, ptr_t ptr )
{
	return _msize( ptr );
}

//======================================================================================
static AssertAction::Kind MyAssertHook( ptr_t context, const AssertInfo_s& info, cstr_t msg )
{
	Debug_PrintLnF("%s(%d): Assertion Failed: %s", info.File, info.Line, info.Expression);
	return AssertAction::Continue;
}

//======================================================================================
static void TestAssertHook()
{
	Assert_SetHook( AssertHook_s { MyAssertHook, nullptr } );
	NeAssert(0 == 1);
}

static void TestAllocHook()
{
	Alloc_s trace_alloc = { TraceAlloc_Realloc, TraceAlloc_SizeOf };

	Mem_SetHook( &trace_alloc );

	Alloc_t alloc = nullptr;
	int* p0 = (int*)Mem_Alloc( alloc, 10 * sizeof(int) );
	int* p1 = (int*)Mem_Calloc( alloc, 10 * sizeof(int) );
	int* p2 = Mem_Alloc<int>( alloc, 10 );
	int* p3 = Mem_Calloc<int>( alloc, 10 );
	Mem_Free( alloc, p3 );
	Mem_Free( alloc, p2 );
	Mem_Free( alloc, p1 );
	Mem_Free( alloc, p0 );

	Mem_SetHook( nullptr );
}

static void TestCountedAlloc()
{
	CountedAlloc_s counted_alloc = CountedAlloc_Create( Alloc_GetDefault(), "Counted Allocator" );
	Alloc_t alloc = &counted_alloc.Alloc;
	int* p0 = (int*)Mem_Alloc( alloc, 10 * sizeof(int) );
	int* p1 = (int*)Mem_Calloc( alloc, 10 * sizeof(int) );
	int* p2 = Mem_Alloc<int>( alloc, 10 );
	int* p3 = Mem_Calloc<int>( alloc, 10 );
	Mem_Free( alloc, p3 );
	Mem_Free( alloc, p2 );
	Mem_Free( alloc, p1 );
	Mem_Free( alloc, p0 );
	CountedAlloc_Dump( &counted_alloc );
}

static void TestCountedAllocHook()
{
	CountedAlloc_s counted_alloc = CountedAlloc_Create( Alloc_GetDefault(), "Counted Allocator Hook" );
	
	Mem_SetHook( &counted_alloc.Alloc );

	Alloc_t alloc = nullptr;
	int* p0 = (int*)Mem_Alloc( alloc, 10 * sizeof(int) );
	int* p1 = (int*)Mem_Calloc( alloc, 10 * sizeof(int) );
	int* p2 = Mem_Alloc<int>( alloc, 10 );
	int* p3 = Mem_Calloc<int>( alloc, 10 );
	Mem_Free( alloc, p3 );
	Mem_Free( alloc, p2 );
	Mem_Free( alloc, p1 );
	Mem_Free( alloc, p0 );

	CountedAlloc_Dump( &counted_alloc );

	Mem_SetHook( nullptr );
}

//======================================================================================
int main( int argc, const char** argv )
{
	TestAssertHook();
	TestAllocHook();
	TestCountedAlloc();
	TestCountedAllocHook();

	return 0;
}
