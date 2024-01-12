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

	template <typename T>
	inline Span<T> Span_Cast( T* item, int count )
	{
		return Span<T> { item, count };
	}

	template <typename T, size_t S>
	inline Span<T> Span_Cast( T (&items)[S] )
	{
		return Span<T> { items, S };
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

}
