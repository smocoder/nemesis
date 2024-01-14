#pragma once

//======================================================================================
#include "Assert.h"

//======================================================================================
namespace Nemesis
{
	template <typename T>
	struct Span
	{
	public:
		T* Item;
		int Count;

	public:
		void Zero();
		void Fill( const T& item );

	public:
		Span<T>			Left ( int count );
		Span<const T>	Left ( int count ) const;
		Span<T>			Right( int count );
		Span<const T>	Right( int count ) const;
		Span<T>			Mid	 ( int index, int count );
		Span<const T>	Mid	 ( int index, int count ) const;

	public:
		T&		 operator [] ( int index );
		const T& operator [] ( int index ) const;

		operator Span<const T> () const;
	};

	//==================================================================================

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

	//==================================================================================

	template <typename T>
	inline void Span<T>::Zero()
	{
		Arr_Zero<T>( Item, Count );
	}

	template <typename T>
	inline void Span<T>::Fill( const T& value )
	{
		Arr_Set<T>( Item, value, Count );
	}

	//==================================================================================

	template <typename T>
	inline Span<T> Span<T>::Left( int count )
	{
		return Mid( 0, count );
	}

	template <typename T>
	inline Span<const T> Span<T>::Left( int count ) const
	{
		return Mid( 0, count );
	}

	template <typename T>
	inline Span<T> Span<T>::Right( int count )
	{
		return Mid( Count - count, count );
	}

	template <typename T>
	inline Span<const T> Span<T>::Right( int count ) const
	{
		return Mid( Count - count, count );
	}

	template <typename T>
	inline Span<T> Span<T>::Mid( int index, int count )
	{
		NeAssertBounds(index, Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		NeAssert((index + count) <= Count);
		return Span<T> { Item + index, count };
	}

	template <typename T>
	inline Span<const T> Span<T>::Mid( int index, int count ) const
	{
		NeAssertBounds(index, Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		NeAssert((index + count) <= Count);
		return Span<const T> { Item + index, count };
	}

	//==================================================================================

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

	//==================================================================================

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

	//==================================================================================

	template <typename T>
	inline T* begin( Span<T>& span )
	{
		return span.Item;
	}

	template <typename T>
	inline const T* begin( const Span<T>& span )
	{
		return span.Item;
	}

	template <typename T>
	inline T* end( Span<T>& span )
	{
		return span.Item + span.Count;
	}

	template <typename T>
	inline const T* end( const Span<T>& span )
	{
		return span.Item + span.Count;
	}

}
