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
	Assert_SetHook();
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

static void Array_Print( cstr_t title, const Array<int>& arr )
{
	printf("%s:\n", title);
	for ( int v : arr )
		printf("%d\n", v);
}

static void Array_SetIndex( Array<int>& arr, int first, int count )
{
	for ( int i = 0; i < count; ++i )
		arr[first+i] = first+i;
}

static void Array_AddIndex( Array<int>& arr, int count )
{
	const int first = arr.Count; 
	for ( int i = 0; i < count; ++i )
		Array_Append( arr, first+i );
}

static void Array_SetRanged( Array<int>& arr, int val )
{
	for ( int& v : arr )
		v = val;
}

static void TestArray()
{
	const int values [] = { 42, 17, 12, 7, 3 };

	Array<int> arr = {};
	Array_Reserve( arr, 20 );								Array_Print( "Reserve", arr );
	Array_Resize( arr, 10 );								Array_Print( "Resize", arr );
	Array_Zero( arr );										Array_Print( "Zero", arr );
	Array_Fill( arr, -1 );									Array_Print( "Fill (-1)", arr );
	Array_SetRanged( arr, -42 );							Array_Print( "SetRanged", arr );
	Array_SetIndex( arr, 0, 10 );							Array_Print( "SetIndex", arr );
	Array_AddIndex( arr, 10 );								Array_Print( "AddIndex", arr );
	Array_RemoveSwapAt( arr, 0 );							Array_Print( "RemoveSwapAt", arr );
	Array_RemoveAt( arr, 0 );								Array_Print( "RemoveAt", arr );
	Array_Reset( arr );										Array_Print( "Reset", arr );
	Array_Append( arr, values, NeCountOf(values) );			Array_Print( "Append (Multi)", arr );
	Array_InsertAt( arr, 1, values, NeCountOf(values) );	Array_Print( "InsertAt (Multi)", arr );
	Array_RemoveAt( arr, 1, NeCountOf(values) );			Array_Print( "RemoveAt (Multi)", arr );
	Array_RemoveAt( arr, 0, arr.Count );					Array_Print( "RemoveAt (All)", arr );
	Array_Resize( arr, 10 );
	Array_SetIndex( arr, 0, 10 );							Array_Print( "SetIndex", arr );
	{
		const int val = 7;
		const int idx = Array_LinearFind( arr, val );
		printf( "LinearFind(%d): %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Array_LinearFind( arr, val );
		printf( "LinearFind(%f): %d\n", val, idx);
	}
	{
		struct Cmp1
		{
			static bool Equals( int a, float b ) { return a == b; }
		};
		const float val = 7.0f;
		const int idx = Array_LinearFind<Cmp1>( arr, val );
		printf( "LinearFind<Comparer>(%f): %d\n", val, idx);
	}
	{
		struct Cmp2
		{
			bool operator() ( int a, float b ) const { return a == b; }
		};

		const float val = 7.0f;
		const int idx = Array_LinearFind( arr, val, Cmp2 {} );
		printf( "LinearFind(Predicate)(%f): %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Array_LinearFind( arr, val, []( int a, float b ) { return a == b; } );
		printf( "LinearFind(Lambda)(%f): %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Array_BinaryFind( arr, val );
		printf( "BinaryFind(%d): %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Array_BinaryFind( arr, val );
		printf( "BinaryFind(%f): %d\n", val, idx);
	}
	{
		struct Cmp3
		{
			static int Compare( int a, float b ) 
			{ 
				if (a == b)
					return 0;
				if (a > b)
					return 1;
				return -1;
			}
		};
		const float val = 7.0f;
		const int idx = Array_BinaryFind<Cmp3>( arr, val );
		printf( "BinaryFind<Comparer>(%f): %d\n", val, idx);
	}
	{
		struct Cmp4
		{
			int operator() ( int a, float b ) const 
			{ 
				if (a == b)
					return 0;
				if (a > b)
					return 1;
				return -1;
			}
		};

		const float val = 7.0f;
		const int idx = Array_BinaryFind( arr, val, Cmp4 {} );
		printf( "BinaryFind(Predicate)(%f): %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Array_BinaryFind
			( arr
			, val
			, []( int a, float b ) 
				{ 
					if (a == b)
						return 0;
					if (a > b)
						return 1;
					return -1;
				} 
			);
		printf( "LinearFind(Lambda)(%f): %d\n", val, idx);
	}
	Array_Clear( arr );
}

static void Span_Print( cstr_t title, const Span<const int>& span )
{
	printf("%s:\n", title);
	for ( int v : span )
		printf("%d\n", v);
}

static void TestSpan()
{
	Array<int> arr = {};
	Array_Resize( arr, 30 );
	{
		// non-const
		Span<int> span_l = Array_SpanLeft ( arr, 10 );
		Span<int> span_r = Array_SpanRight( arr, 10 );
		Span<int> span_m = Array_SpanMid  ( arr, 10, arr.Count - span_l.Count - span_r.Count );
		Span_Fill( span_l, 42 );
		Span_Fill( span_r, 17 );
		Span_Zero( span_m );
		Array_Print( "Fill (Span)", arr );
	}
	{
		// const conversion operator
		const Span<const int> span_l = Array_SpanLeft ( arr, 10 );
		const Span<const int> span_r = Array_SpanRight( arr, 10 );
		const Span<const int> span_m = Array_SpanMid  ( arr, 10, arr.Count - span_l.Count - span_r.Count );
		Span_Print( "SpanLeft" , span_l );
		Span_Print( "SpanRight", span_r );
		Span_Print( "SpanMid"  , span_m );
	}
	{
		// const overload
		const Array<int>& const_arr = arr;
		const Span<const int> span_l = Array_SpanLeft ( const_arr, 10 );
		const Span<const int> span_r = Array_SpanRight( const_arr, 10 );
		const Span<const int> span_m = Array_SpanMid  ( const_arr, 10, arr.Count - span_l.Count - span_r.Count );
		Span_Print( "SpanLeft" , span_l );
		Span_Print( "SpanRight", span_r );
		Span_Print( "SpanMid"  , span_m );
	}
}

//======================================================================================
int main( int argc, const char** argv )
{
	TestAssertHook();
	TestAllocHook();
	TestCountedAlloc();
	TestCountedAllocHook();
	TestArray();
	TestSpan();

	return 0;
}
