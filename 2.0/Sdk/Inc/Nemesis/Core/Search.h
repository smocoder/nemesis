#pragma once

//======================================================================================
namespace Nemesis
{
	// Locates a matching item within the array using linear search.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns -1.
	// This overload uses comparison operators for comparison.
	template <typename T, typename U>
	inline int Find_Linear( const T& a, const U& v )
	{
		for ( int i = 0; i < a.Count; ++i )
		{
			if (a[i] == v)
				return i;
		}
		return -1;
	}

	// Locates a matching item within the array using linear search.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns -1.
	// This overload uses a comparer for comparison.
	template <typename Comparer, typename T, typename U>
	inline int Find_Linear( const T& a, const U& v )
	{
		for ( int i = 0; i < a.Count; ++i )
		{
			if (Comparer::Equals(a[i], v))
				return i;
		}
		return -1;
	}

	// Locates a matching item within the array using linear search.
	// If a match is found, the function returns its index.
	// If no match is found, the function returns -1.
	// This overload uses a predicate for comparison.
	template <typename T, typename U, typename Predicate>
	inline int Find_Linear( const T& a, const U& v, const Predicate& predicate )
	{
		for ( int i = 0; i < a.Count; ++i )
		{
			if (predicate(a[i], v))
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
	inline int Find_Binary( const T& a, const U& v )
	{
		int pivot;
		int start = 0;
		int end = a.Count-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			if (a[pivot] == v)
				return pivot;
			if (a[pivot] > v)
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
	inline int Find_Binary( const T& a, const U& v )
	{
		int cmp;
		int pivot;
		int start = 0;
		int end = a.Count-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			cmp = Comparer::Compare( a[pivot], v );
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
	inline int Find_Binary( const T& a, const U& v, const Predicate& predicate )
	{
		int cmp;
		int pivot;
		int start = 0;
		int end = a.Count-1;
		for ( ; start <= end; )
		{
			pivot = (start+end)/2;
			cmp = predicate( a[pivot], v );
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
