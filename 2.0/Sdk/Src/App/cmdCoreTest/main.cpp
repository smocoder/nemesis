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

static void TestSearch()
{
	Array<int> arr = {};
	Array_Resize( arr, 10 );
	Array_SetIndex( arr, 0, 10 );

	const Span<const int> span = Array_Span( arr );

	// --- Operators

	{
		const int val = 7;
		const int idx = Find_Linear( arr, val );
		printf( "Find (Linear, Int, Intrinsic, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear( span, val );
		printf( "Find (Linear, Int, Intrinsic, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear( span.Item, span.Count, val );
		printf( "Find (Linear, Int, Intrinsic, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( arr, val );
		printf( "Find (Linear, Float, Intrinsic, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( span, val );
		printf( "Find (Linear, Float, Intrinsic, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( span.Item, span.Count, val );
		printf( "Find (Linear, Float, Intrinsic, Plain): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( arr, val );
		printf( "Find (Binary, Int, Intrinsic, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( span, val );
		printf( "Find (Binary, Int, Intrinsic, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( span.Item, span.Count, val );
		printf( "Find (Binary, Int, Intrinsic, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary( arr, val );
		printf( "Find (Binary, Float, Intrinsic, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary( span, val );
		printf( "Find (Binary, Float, Intrinsic, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary( span.Item, span.Count, val );
		printf( "Find (Binary, Float, Intrinsic, Plain): Val: %f -> Idx: %d\n", val, idx);
	}
	 
	// --- Comparer

	struct Comparer
	{
		static bool Equals( int a, int   b ) { return a == b; }
		static bool Equals( int a, float b ) { return a == b; }
		static int Compare( int a, int b )   { return a-b; }
		static int Compare( int a, float b ) 
		{ 
			if (a == b)
				return 0;
			if (a > b)
				return 1;
			return -1;
		}
	};

	{
		const int val = 7;
		const int idx = Find_Linear<Comparer>( arr, val );
		printf( "Find (Linear, Int, Comparer, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear<Comparer>( span, val );
		printf( "Find (Linear, Int, Comparer, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear<Comparer>( span.Item, span.Count, val );
		printf( "Find (Linear, Int, Comparer, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear<Comparer>( arr, val );
		printf( "Find (Linear, Float, Comparer, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear<Comparer>( span, val );
		printf( "Find (Linear, Float, Comparer, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear<Comparer>( span.Item, span.Count, val );
		printf( "Find (Linear, Float, Comparer, Plain): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary<Comparer>( arr, val );
		printf( "Find (Binary, Int, Comparer, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary<Comparer>( span, val );
		printf( "Find (Binary, Int, Comparer, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary<Comparer>( span.Item, span.Count, val );
		printf( "Find (Binary, Int, Comparer, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary<Comparer>( arr, val );
		printf( "Find (Binary, Float, Comparer, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary<Comparer>( span, val );
		printf( "Find (Binary, Float, Comparer, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary<Comparer>( span.Item, span.Count, val );
		printf( "Find (Binary, Float, Comparer, Plain): Val: %f -> Idx: %d\n", val, idx);
	}
	 
	// --- Predicate

	struct Equals
	{
		bool operator () ( int a, int   b ) const { return a == b; }
		bool operator () ( int a, float b ) const { return a == b; }
	};

	struct Compare
	{
		int operator () ( int a, int b )   const { return a-b; }
		int operator () ( int a, float b ) const
		{ 
			if (a == b)
				return 0;
			if (a > b)
				return 1;
			return -1;
		}
	};

	{
		const int val = 7;
		const int idx = Find_Linear( arr, val, Equals {} );
		printf( "Find (Linear, Int, Predicate, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear( span, val, Equals {} );
		printf( "Find (Linear, Int, Predicate, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear( span.Item, span.Count, val, Equals {} );
		printf( "Find (Linear, Int, Predicate, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( arr, val, Equals {} );
		printf( "Find (Linear, Float, Predicate, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( span, val, Equals {} );
		printf( "Find (Linear, Float, Predicate, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( span.Item, span.Count, val, Equals {} );
		printf( "Find (Linear, Float, Predicate, Plain): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( arr, val, Compare {} );
		printf( "Find (Binary, Int, Predicate, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( span, val, Compare {} );
		printf( "Find (Binary, Int, Predicate, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( span.Item, span.Count, val, Compare {} );
		printf( "Find (Binary, Int, Predicate, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary( arr, val, Compare {} );
		printf( "Find (Binary, Float, Predicate, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary( span, val, Compare {} );
		printf( "Find (Binary, Float, Predicate, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary( span.Item, span.Count, val, Compare {} );
		printf( "Find (Binary, Float, Predicate, Plain): Val: %f -> Idx: %d\n", val, idx);
	}

	// --- Lambda

	{
		const int val = 7;
		const int idx = Find_Linear( arr, val, [] ( int a, int b ) { return a == b; } );
		printf( "Find (Linear, Int, Lambda, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear( span, val, [] ( int a, int b ) { return a == b; } );
		printf( "Find (Linear, Int, Lambda, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Linear( span.Item, span.Count, val, [] ( int a, int b ) { return a == b; } );
		printf( "Find (Linear, Int, Lambda, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( arr, val, [] ( int a, float b ) { return a == b; } );
		printf( "Find (Linear, Float, Lambda, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( span, val, [] ( int a, float b ) { return a == b; } );
		printf( "Find (Linear, Float, Lambda, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Linear( span.Item, span.Count, val, [] ( int a, float b ) { return a == b; } );
		printf( "Find (Linear, Float, Lambda, Plain): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( arr, val, [] ( int a, int b ) { return a-b; } );
		printf( "Find (Binary, Int, Lambda, Array): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( span, val, [] ( int a, int b ) { return a-b; } );
		printf( "Find (Binary, Int, Lambda, Span): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const int val = 7;
		const int idx = Find_Binary( span.Item, span.Count, val, [] ( int a, int b ) { return a-b; } );
		printf( "Find (Binary, Int, Lambda, Plain): Val: %d -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary
			( arr
			, val
			, [] ( int a, float b )
				{
					if (a == b)
						return 0;
					if (a > b)
						return 1;
					return -1;
				} 
			);
		printf( "Find (Binary, Float, Lambda, Array): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary
			( span
			, val
			, [] ( int a, float b )
				{
					if (a == b)
						return 0;
					if (a > b)
						return 1;
					return -1;
				} 
			);
		printf( "Find (Binary, Float, Lambda, Span): Val: %f -> Idx: %d\n", val, idx);
	}
	{
		const float val = 7.0f;
		const int idx = Find_Binary
			( span.Item
			, span.Count
			, val
			, [] ( int a, float b )
				{
					if (a == b)
						return 0;
					if (a > b)
						return 1;
					return -1;
				} 
			);
		printf( "Find (Binary, Float, Lambda, Plain): Val: %f -> Idx: %d\n", val, idx);
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
	TestSearch();

	return 0;
}
