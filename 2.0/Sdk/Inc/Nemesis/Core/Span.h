#pragma once

//======================================================================================
#include "Assert.h"

//======================================================================================
namespace Nemesis
{
	template <typename T>
	struct Span
	{
		T* Item;
		int Count;

		T&		 operator [] ( int index );
		const T& operator [] ( int index ) const;

		operator Span<const T> () const;
	};
}

//======================================================================================
namespace Nemesis
{
	template <typename T>
	inline T& Span<T>::operator [] ( int index )	
	{ 
		NeAssertBounds(index, Count); 
		return Item[index]; 
	}

	template <typename T>
	inline const T& Span<T>::operator [] ( int index ) const 
	{ 
		NeAssertBounds(index, Count); 
		return Item[index]; 
	}

	template <typename T>
	inline Span<T>::operator Span<const T> () const 
	{ 
		return Span<const T> { Item, Count }; 
	}

}

//======================================================================================
namespace Nemesis
{
	template <typename T>
	T* begin( Span<T>& span )
	{
		return span.Item;
	}

	template <typename T>
	const T* begin( const Span<T>& span )
	{
		return span.Item;
	}

	template <typename T>
	T* end( Span<T>& span )
	{
		return span.Item + span.Count;
	}

	template <typename T>
	const T* end( const Span<T>& span )
	{
		return span.Item + span.Count;
	}
}

//======================================================================================
namespace Nemesis
{
	//----------------------------------------------------------------------------------

	template <typename T>
	inline void Span_Init( Span<T>& span, T* item, int count )
	{
		NeAssert(item || !count);
		span.Item = item;
		span.Count = count;
	}

	//----------------------------------------------------------------------------------

	template <typename T>
	inline void Span_Zero( Span<T>& span )
	{
		Arr_Zero<T>( span.Item, span.Count );
	}

	template <typename T>
	inline void Span_Fill( Span<T>& span, const T& value )
	{
		Arr_Set<T>( span.Item, value, span.Count );
	}

	//----------------------------------------------------------------------------------

	template <typename T>
	inline bool Span_IsValidIndex( const Span<T>& span, int index )
	{
		return (index >= 0) && (index < span.Count);
	}

	template <typename T>
	inline size_t Span_GetItemSize( const Span<T>& span )
	{
		return sizeof(T);
	}

	template <typename T>
	inline size_t Span_GetUsedSize( const Span<T>& span )
	{
		return span.Count * sizeof(T);
	}

	template <typename T>
	inline size_t Span_GetReservedSize( const Span<T>& span )
	{
		return span.Capacity * sizeof(T);
	}

	//----------------------------------------------------------------------------------

	// Locates a matching item within the array using linear search.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns -1.
	// This overload uses comparison operators for comparison.
	template <typename T, typename U>
	inline int Span_LinearFind( const Span<T>& span, const U& v )
	{
		for ( int i = 0; i < span.Count; ++i )
		{
			if (span[i] == v)
				return i;
		}
		return -1;
	}

	// Locates a matching item within the array using linear search.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns -1.
	// This overload uses a comparer for comparison.
	template <typename Comparer, typename T, typename U>
	inline int Span_LinearFind( const Span<T>& span, const U& v )
	{
		for ( int i = 0; i < span.Count; ++i )
		{
			if (Comparer::Equals(span[i], v))
				return i;
		}
		return -1;
	}

	// Locates a matching item within the array using linear search.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns -1.
	// This overload uses a predicate for comparison.
	template <typename T, typename U, typename Predicate>
	inline int Span_LinearFind( const Span<T>& span, const U& v, const Predicate& predicate )
	{
		for ( int i = 0; i < span.Count; ++i )
		{
			if (predicate(span[i], v))
				return i;
		}
		return -1;
	}

	// Locates a matching item within the array using binary search.
	// The array must be sorted.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns the two's complement
	// of the index where the value should be inserted. 
	// This overload uses comparison operators for comparison.
	template <typename T, typename U>
	inline int Span_BinaryFind( const Span<T>& span, const U& v )
	{
		int pivot;
		int start = 0;
		int end = span.Count-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			if (span[pivot] == v)
				return pivot;
			if (span[pivot] > v)
				end = pivot-1;
			else
				start = pivot+1;
		}
		return ~start;
	}

	// Locates a matching item within the array using binary search.
	// The array must be sorted.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns the two's complement
	// of the index where the value should be inserted. 
	// This overload uses a comparer for comparisons.
	template <typename Comparer, typename T, typename U>
	inline int Span_BinaryFind( const Span<T>& span, const U& v )
	{
		int cmp;
		int pivot;
		int start = 0;
		int end = span.Count-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			cmp = Comparer::Compare( span[pivot], v );
			if (0 == cmp)
				return pivot;
			if (cmp > 0)
				end = pivot-1;
			else
				start = pivot+1;
		}
		return ~start;
	}

	// Locates a matching item within the array using binary search.
	// The array must be sorted.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns the two's complement
	// of the index where the value should be inserted. 
	// This overload uses a predicate for comparisons.
	template <typename T, typename U, typename Predicate>
	inline int Span_BinaryFind( const Span<T>& span, const U& v, const Predicate& predicate )
	{
		int cmp;
		int pivot;
		int start = 0;
		int end = span.Count-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			cmp = predicate( span[pivot], v );
			if (0 == cmp)
				return pivot;
			if (cmp > 0)
				end = pivot-1;
			else
				start = pivot+1;
		}
		return ~start;
	}

}