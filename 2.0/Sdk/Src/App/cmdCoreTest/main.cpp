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

static Array<int> Array_Build()
{
	Array<int> arr = {};
	Array_Resize( arr, 10 );
	Array_SetIndex( arr, 0, 10 );
	return arr;
}

static void TestArray()
{
	const int values [] = { 42, 17, 12, 7, 3 };

	Array<int> arr;
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

	Array<int> arr1;
	Array_Resize( arr1, 10 );
	Array_SetIndex( arr1, 0, 10 );

	Array<int> arr2 = arr1;
	Array_RemoveAt( arr1, 0, arr1.Count );

	Array<int> arr3 = Array_Build();
	Array_Print( "Move Ctor", arr3 );

	Array<int>&& arr4 = {};
	arr4.operator=(static_cast<Array<int>&&>(arr3));
	Array_Print( "Move Operator", arr4 );
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

namespace
{
	static const uint32_t ANIM_TOKEN_ASSET = NeMakeFourCc( 'N', 'E', 'A', 'S' );
	static const uint32_t ANIM_TOKEN_ARIG  = NeMakeFourCc( 'A', 'R', 'I', 'G' );
}

static void TestTable()
{
	struct AnimVec3_s
	{
		float x, y, z, _;
	};

	struct AnimQuat_s
	{
		float x, y, z, w;
	};

	struct AnimAsset_s
	{
		uint32_t Size;
		uint32_t Token1;
		uint32_t Token2;
		uint32_t _pad_;
	};

	struct AnimRig_s
	{
		AnimAsset_s		  Asset;
		Table<AnimVec3_s> BindPos;
		Table<AnimQuat_s> BindRot;
		Table<int>		  Parents;
		Table<char>		  FileName;
		NameTable_s		  BoneNames;
	};

	struct AnimRigDesc_s
	{
		Span<const AnimVec3_s>	BindPos;
		Span<const AnimQuat_s>	BindRot;
		Span<const int>			Parents;
		Span<const cstr_t>		BoneNames;
		cstr_t					FileName;
	};

	struct AnimBuffer_s
	{
		Alloc_t  Alloc;
		ptr_t    Data;
		uint32_t Size;
	};

	struct AnimTool
	{
		static void Setup( AnimRig_s* rig, const AnimRigDesc_s& desc, TableSetup::Mode mode )
		{
			NeStaticAssert((sizeof(*rig) % 16) == 0);

			const int file_name_len = 1+Str_Len( desc.FileName );

			uint8_t* pos = (uint8_t*)(rig+1);
			pos = Table_Setup	 ( &rig->BindPos  , pos, desc.BindPos				 , mode );
			pos = Table_Setup	 ( &rig->BindRot  , pos, desc.BindRot				 , mode );
			pos = Table_Setup	 ( &rig->Parents  , pos, desc.Parents				 , mode );
			pos = Table_Setup	 ( &rig->FileName , pos, desc.FileName, file_name_len, mode );
			pos = NameTable_Setup( &rig->BoneNames, pos, desc.BoneNames				 , mode );

			rig->Asset = AnimAsset_s { (uint32_t)Ptr_Tell( rig, pos ), ANIM_TOKEN_ASSET, ANIM_TOKEN_ARIG };
		}

		static void ReBase( AnimRig_s* rig, ReBase::Op op )
		{
			Table_ReBase	( &rig->BindPos	 , op );
			Table_ReBase	( &rig->BindRot	 , op );
			Table_ReBase	( &rig->Parents	 , op );
			Table_ReBase	( &rig->FileName , op );
			NameTable_ReBase( &rig->BoneNames, op );
		}

		static uint32_t CalcSize( const AnimRigDesc_s& desc )
		{
			AnimRig_s rig = {};
			Setup( &rig, desc, TableSetup::CalcSize );
			return rig.Asset.Size;
		}

		static AnimRig_s* Create( const AnimRigDesc_s& desc, ptr_t data, uint32_t size )
		{
			NeAssert(size >= CalcSize( desc ));
			AnimRig_s* rig = (AnimRig_s*)data;
			Setup( rig, desc, TableSetup::CopyData );
			return rig;
		}

		static AnimBuffer_s Compile( Alloc_t alloc, const AnimRigDesc_s& desc )
		{
			const uint32_t size = CalcSize( desc );
			AnimRig_s* rig = Create( desc, Mem_Calloc( alloc, size ), size );
			ReBase( rig, ReBase::Detach );
			return AnimBuffer_s { alloc, rig, size };
		}

		static void Locate( ptr_t data, uint32_t size, AnimRig_s** out )
		{
			AnimRig_s* rig = (AnimRig_s*)data;
			NeAssert(rig->Asset.Size <= size);
			NeAssert(rig->Asset.Token1 == ANIM_TOKEN_ASSET);
			NeAssert(rig->Asset.Token2 == ANIM_TOKEN_ARIG);
			ReBase( rig, ReBase::Attach );
			*out = rig;
		}

		static void Locate( const AnimBuffer_s& buffer, AnimRig_s** out )
		{
			return Locate( buffer.Data, buffer.Size, out );
		}

		static void Free( const AnimBuffer_s& buffer )
		{
			Mem_Free( buffer.Alloc, buffer.Data );
		}

		static void Print( const AnimRig_s* rig )
		{
			printf( "Rig Name...: %s\n", rig->FileName.Item );
			printf( "  Num Pos....: %d\n", rig->BindPos.Count );
			printf( "  Num Rot....: %d\n", rig->BindRot.Count );
			printf( "  Num Parents: %d\n", rig->Parents.Count );
			printf( "  Positions:\n" );
			for ( const AnimVec3_s& v : rig->BindPos )
				printf( "    {%5.2f %5.2f %5.2f}\n", v.x, v.y, v.z );
			printf( "  Rotations:\n" );
			for ( const AnimQuat_s& v : rig->BindRot )
				printf( "    {%5.2f %5.2f %5.2f %5.2f}\n", v.x, v.y, v.z, v.w );
			printf( "  Hierarchy:\n" );
			for ( int v : rig->Parents )
				printf( "    %+2d\n", v );
			printf( "  Bones:\n" );
			const int num_bone_names = NameTable_GetNumNames( &rig->BoneNames );
			for ( int i = 0; i < num_bone_names; ++i )
				printf( "    %s\n", NameTable_GetName( &rig->BoneNames, i ) );
		}

	};

	const Alloc_t	 alloc			= nullptr;
	const AnimQuat_s rot_id			= { 0.0f, 0.0f, 0.0f, 1.0f };
	const AnimVec3_s bone_pos	[]	= { { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } };
	const AnimQuat_s bone_rot	[]	= { { rot_id }, { rot_id }, { rot_id } };
	const int		 parents	[]	= { -1, 0, 1 };
	const cstr_t	 bone_names	[]	= { "Root", "Hips", "Head" }; 

	// create from desc
	{
		const AnimRigDesc_s rig_desc = 
			{ Span_Cast(bone_pos)
			, Span_Cast(bone_rot)
			, Span_Cast(parents)
			, Span_Cast(bone_names)
			, "CreateFromDesc"
			};

		const uint32_t rig_size = AnimTool::CalcSize( rig_desc );
		AnimRig_s* rig = AnimTool::Create( rig_desc, Mem_Calloc( alloc, rig_size ), rig_size );
		NeAssert(rig && (rig->Asset.Size == rig_size));
		AnimTool::Print(rig);
		Mem_Free( alloc, rig );
	}
	
	// compile from desc and locate
	{
		const AnimRigDesc_s rig_desc = 
			{ Span_Cast(bone_pos)
			, Span_Cast(bone_rot)
			, Span_Cast(parents)
			, Span_Cast(bone_names)
			, "CompileAndLocate"
			};

		const AnimBuffer_s rig_buffer = AnimTool::Compile( alloc, rig_desc );
		AnimRig_s* rig = nullptr;
		AnimTool::Locate( rig_buffer, &rig );
		NeAssert(rig && (rig->Asset.Size == rig_buffer.Size));
		AnimTool::Print(rig);
		AnimTool::Free( rig_buffer );
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
	TestTable();

	return 0;
}
