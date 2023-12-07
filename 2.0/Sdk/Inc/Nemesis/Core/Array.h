#pragma once

//======================================================================================
#include "Span.h"
#include "Memory.h"

//======================================================================================
namespace Nemesis
{
	template <typename T>
	struct Array
	{
		T* Item;
		int Count;
		int Capacity;
		Alloc_t Alloc;

		T&		 operator [] ( int index );
		const T& operator [] ( int index ) const;
	};
}

//======================================================================================
namespace Nemesis
{
	template <typename T>
	inline T& Array<T>::operator [] ( int index )	
	{ 
		NeAssertBounds(index, Count); 
		return Item[index]; 
	}

	template <typename T>
	inline const T& Array<T>::operator [] ( int index ) const 
	{ 
		NeAssertBounds(index, Count); 
		return Item[index]; 
	}
}

//======================================================================================
namespace Nemesis
{
	template <typename T>
	T* begin( Array<T>& a )
	{
		return a.Item;
	}

	template <typename T>
	const T* begin( const Array<T>& a )
	{
		return a.Item;
	}

	template <typename T>
	T* end( Array<T>& a )
	{
		return a.Item + a.Count;
	}

	template <typename T>
	const T* end( const Array<T>& a )
	{
		return a.Item + a.Count;
	}
}

//======================================================================================
namespace Nemesis
{
	//----------------------------------------------------------------------------------

	template <typename T>
	inline void Array_Init( Array<T>& a, Alloc_t alloc )
	{
		a.Item = nullptr;
		a.Count = 0;
		a.Capacity = 0;
		a.Alloc = alloc;
	}

	template <typename T>
	inline void Array_Reset( Array<T>& a )
	{
		a.Count = 0;
	}

	template <typename T>
	inline void Array_Clear( Array<T>& a )
	{
		Mem_Free( a.Alloc, a.Item );
		Array_Init( a, a.Alloc );
	}

	template <typename T>
	inline void Array_Reserve( Array<T>& a, int capacity )
	{
		NeAssertOut(capacity >= 0, "Invalid capacity: %d", capacity);
		if (capacity < a.Capacity)
			return;
		a.Item = Arr_Realloc<T>( a.Alloc, a.Item, capacity );
		a.Capacity = capacity;
	}

	template <typename T>
	inline void Array_Resize( Array<T>& a, int count )
	{
		Array_Reserve( a, count );
		a.Count = count;
	}

	template <typename T>
	inline void Array_GrowBy( Array<T>& a, int count )
	{
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		const int new_count = a.Count + count;
		if (new_count > a.Capacity)
		{
			const int grow_by = NeMax( a.Capacity >> 3, 4 );
			const int new_capacity = a.Capacity + grow_by + count;
			Array_Reserve( a, new_capacity );
		}
		a.Count = new_count;
	}

	//----------------------------------------------------------------------------------

	template <typename T>
	inline void Array_Zero( Array<T>& a )
	{
		Arr_Zero<T>( a.Item, a.Count );
	}

	template <typename T>
	inline void Array_Fill( Array<T>& a, const T& value )
	{
		Arr_Set<T>( a.Item, value, a.Count );
	}

	//----------------------------------------------------------------------------------

	template <typename T>
	inline T* Array_Append( Array<T>& a, const T* item, int count )
	{
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		if (!item || !count)
			return nullptr;
		const int index = a.Count;
		Array_GrowBy( a, count );
		Arr_Cpy<T>( a.Item + index, item, count );
		return a.Item + index;
	}

	template <typename T>
	inline T* Array_Append( Array<T>& a, const Array<T>& other )
	{
		return Array_Append( a, other.Item, other.Count );
	}

	template <typename T>
	inline void Array_Append( Array<T>& a, const T& value )
	{
		Array_Append( a, &value, 1 );
	}

	template <typename T>
	inline T* Array_InsertAt( Array<T>& a, int index, const T* item, int count )
	{
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		if (!item || !count)
			return nullptr;
		if (index == a.Count)
			return Array_Append( a, item, count );
		Array_GrowBy( a, count );
		const int end = index + count;
		Arr_Mov<T>( a.Item + end, a.Item + index, a.Count - end );
		Arr_Cpy<T>( a.Item + index, item, count );
		return a.Item + index;
	}

	template <typename T>
	inline T* Array_InsertAt( Array<T>& a, int index, const Array<T>& other )
	{
		return Array_InsertAt( a, index, other.Item, other.Count );
	}

	template <typename T>
	inline void Array_InsertAt( Array<T>& a, int index, const T& item )
	{
		Array_InsertAt( a, index, &item, 1 );
	}

	template <typename T>
	inline void Array_RemoveAt( Array<T>& a, int index, int count )
	{
		NeAssertBounds(index, a.Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		if (!count)
			return;
		const int end = index + count;
		NeAssert(end <= a.Count);
		Arr_Mov<T>( a.Item + index, a.Item + end, a.Count - end );
		a.Count -= count;
	}

	template <typename T>
	inline void Array_RemoveAt( Array<T>& a, int index )
	{
		Array_RemoveAt( a, index, 1 );
	}

	template <typename T>
	inline void Array_RemoveSwapAt( Array<T>& a, int index )
	{
		NeAssertBounds(index, a.Count);
		const int last = a.Count-1;
		if ((last > 0) && (index != last))
			a.Item[index] = a.Item[last];
		a.Count--;
	}

	//----------------------------------------------------------------------------------

	template <typename T>
	inline bool Array_IsValidIndex( const Array<T>& a, int index )
	{
		return (index >= 0) && (index < a.Count);
	}

	template <typename T>
	inline size_t Array_GetItemSize( const Array<T>& a )
	{
		return sizeof(T);
	}

	template <typename T>
	inline size_t Array_GetUsedSize( const Array<T>& a )
	{
		return a.Count * sizeof(T);
	}

	template <typename T>
	inline size_t Array_GetReservedSize( const Array<T>& a )
	{
		return a.Capacity * sizeof(T);
	}

	//----------------------------------------------------------------------------------

	template <typename T>
	inline Span<T> Array_SpanMid( Array<T>& a, int first, int count )
	{
		NeAssertBounds(first, a.Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		NeAssert((first + count) <= a.Count);
		return Span<T> { a.Item + first, count };
	}

	template <typename T>
	inline Span<const T> Array_SpanMid( const Array<T>& a, int first, int count )
	{
		NeAssertBounds(first, a.Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		NeAssert((first + count) <= a.Count);
		return Span<const T> { a.Item + first, count };
	}

	template <typename T>
	inline Span<T> Array_SpanLeft( Array<T>& a, int count )
	{
		return Array_SpanMid( a, 0, count );
	}

	template <typename T>
	inline Span<const T> Array_SpanLeft( const Array<T>& a, int count )
	{
		return Array_SpanMid( a, 0, count );
	}

	template <typename T>
	inline Span<T> Array_SpanRight( Array<T>& a, int count )
	{
		return Array_SpanMid( a, a.Count - count, count );
	}

	template <typename T>
	inline Span<const T> Array_SpanRight( const Array<T>& a, int count )
	{
		return Array_SpanMid( a, a.Count - count, count );
	}

	template <typename T>
	inline Span<T> Array_Span( Array<T>& a )
	{
		return Array_SpanMid( a, 0, a.Count );
	}

	template <typename T>
	inline Span<const T> Array_Span( const Array<T>& a )
	{
		return Array_SpanMid( a, 0, a.Count );
	}

}
