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
	public:
		explicit Array( Alloc_t alloc ); 
		Array( const Array<T>& rhs );
		Array( Array<T>&& rhs );
		Array();
		~Array();

	public:
		void Init( Alloc_t alloc );
		void Clear();
		void Reset();
		void Reserve( int capacity );
		void Resize( int count );
		void GrowBy( int count );

	public:
		void Zero();
		void Fill( const T& item );
		void Copy( const T* item, int count );
		void Copy( const Span<const T>& items );
		void SetAt( int index, const T& item );
		void SetAt( int index, const T* item, int count );
		void SetAt( int index, const Span<const T>& items );

	public:
		void Append( const T& item );
		void Append( const T* item, int count );
		void Append( const Span<const T>& items );
		void InsertAt( int index, const T& item );
		void InsertAt( int index, const T* item, int count );
		void InsertAt( int index, const Span<const T>& items );
		void RemoveAt( int index );
		void RemoveAt( int index, int count );
		void RemoveSwapAt( int index );

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

		Array<T>& operator = ( const Array<T>& rhs );
		Array<T>& operator = ( Array<T>&& rhs );

		operator Span<T>		();
		operator Span<const T>	() const;

	public:
		T* Item;
		int Count;
		int Capacity;
		Alloc_t Alloc;
	};

	//==================================================================================

	template <typename T>
	Array<T>::Array( Alloc_t alloc )
		: Item( nullptr )
		, Count( 0 )
		, Capacity( 0 )
		, Alloc( alloc )
	{}

	template <typename T>
	Array<T>::Array( const Array<T>& rhs )
		: Item( nullptr )
		, Count( 0 )
		, Capacity( 0 )
		, Alloc( rhs.Alloc )
	{
		operator = (rhs);
	}

	template <typename T>
	Array<T>::Array( Array<T>&& rhs )
		: Item( rhs.Item )
		, Count( rhs.Count )
		, Capacity( rhs.Capacity )
		, Alloc( rhs.Alloc )
	{
		rhs.Init( nullptr );
	}

	template <typename T>
	Array<T>::Array()
		: Item( nullptr )
		, Count( 0 )
		, Capacity( 0 )
		, Alloc( nullptr )
	{}

	template <typename T>
	Array<T>::~Array()
	{
		Clear();
	}

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

	template <typename T>
	inline Array<T>& Array<T>::operator = ( const Array<T>& rhs )
	{
		Copy( rhs );
		return *this;
	}

	template <typename T>
	inline Array<T>& Array<T>::operator = ( Array<T>&& rhs )
	{
		Item = rhs.Item;
		Count = rhs.Count;
		Capacity = rhs.Capacity;
		Alloc = rhs.Alloc;
		rhs.Init( nullptr );
		return *this;
	}

	template <typename T>
	inline Array<T>::operator Span<T>()
	{
		return Span<T> { Item, Count };
	}

	template <typename T>
	inline Array<T>::operator Span<const T>() const
	{
		return Span<const T> { Item, Count };
	}

	//==================================================================================

	template <typename T>
	inline void Array<T>::Init( Alloc_t alloc )
	{
		Item = nullptr;
		Count = 0;
		Capacity = 0;
		Alloc = alloc;
	}

	template <typename T>
	inline void Array<T>::Clear()
	{
		Mem_Free( Alloc, Item );
		Init( Alloc );
	}

	template <typename T>
	inline void Array<T>::Reset()
	{
		Count = 0;
	}

	template <typename T>
	inline void Array<T>::Reserve( int capacity )
	{
		NeAssertOut(capacity >= 0, "Invalid capacity: %d", capacity);
		if (capacity < Capacity)
			return;
		Item = Arr_Realloc<T>( Alloc, Item, capacity );
		Capacity = capacity;
	}

	template <typename T>
	inline void Array<T>::Resize( int count )
	{
		Reserve( count );
		Count = count;
	}

	template <typename T>
	inline void Array<T>::GrowBy( int count )
	{
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		const int new_count = Count + count;
		if (new_count > Capacity)
		{
			const int grow_by = NeMax( Capacity >> 3, 4 );
			const int new_capacity = Capacity + grow_by + count;
			Reserve( new_capacity );
		}
		Count = new_count;
	}

	//==================================================================================

	template <typename T>
	inline void Array<T>::Zero()
	{
		Arr_Zero<T>( Item, Count );
	}

	template <typename T>
	inline void Array<T>::Fill( const T& item )
	{
		Arr_Set<T>( Item, item, Count );
	}

	template <typename T>
	inline void Array<T>::Copy( const T* items, int count )
	{
		Resize( count );
		SetAt( 0, items, count );
	}

	template <typename T>
	inline void Array<T>::Copy( const Span<const T>& items )
	{
		Copy( items.Item, items.Count );
	}

	template <typename T>
	inline void Array<T>::SetAt( int index, const T& item )
	{
		NeAssertBounds(index, Count);
		Item[index] = item;
	}

	template <typename T>
	inline void Array<T>::SetAt( int index, const T* item, int count )
	{
		NeAssertBounds(index, Count);
		NeAssert((index + count) <= Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		Arr_Cpy( Item + index, item, count );
	}

	template <typename T>
	inline void Array<T>::SetAt( int index, const Span<const T>& items )
	{
		SetAt( index, items.Item, items.Count );
	}

	//==================================================================================

	template <typename T>
	inline void Array<T>::Append( const T& item )
	{
		Append( &item, 1 );
	}

	template <typename T>
	inline void Array<T>::Append( const T* item, int count )
	{
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		if (!item || !count)
			return;
		const int index = Count;
		GrowBy( count );
		Arr_Cpy<T>( Item + index, item, count );
	}

	template <typename T>
	inline void Array<T>::Append( const Span<const T>& items )
	{
		Append( items.Item, items.Count );
	}

	template <typename T>
	inline void Array<T>::InsertAt( int index, const T& item )
	{
		InsertAt( index, &item, 1 );
	}

	template <typename T>
	inline void Array<T>::InsertAt( int index, const T* item, int count )
	{
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		if (!item || !count)
			return;
		if (index == Count)
			return Append( item, count );
		GrowBy( count );
		const int end = index + count;
		Arr_Mov<T>( Item + end, Item + index, Count - end );
		Arr_Cpy<T>( Item + index, item, count );
	}

	template <typename T>
	inline void Array<T>::InsertAt( int index, const Span<const T>& items )
	{
		InsertAt( index, items.Item, items.Count );
	}

	template <typename T>
	inline void Array<T>::RemoveAt( int index )
	{
		RemoveAt( index, 1 );
	}

	template <typename T>
	inline void Array<T>::RemoveAt( int index, int count )
	{
		NeAssertBounds(index, Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		if (!count)
			return;
		const int end = index + count;
		NeAssert(end <= Count);
		Arr_Mov<T>( Item + index, Item + end, Count - end );
		Count -= count;
	}

	template <typename T>
	inline void Array<T>::RemoveSwapAt( int index )
	{
		NeAssertBounds(index, Count);
		const int last = Count-1;
		if ((last > 0) && (index != last))
			Item[index] = Item[last];
		Count--;
	}

	//==================================================================================

	template <typename T>
	inline Span<T> Array<T>::Left( int count )
	{
		return Mid( 0, count );
	}

	template <typename T>
	inline Span<const T> Array<T>::Left( int count ) const
	{
		return Mid( 0, count );
	}

	template <typename T>
	inline Span<T> Array<T>::Right( int count )
	{
		return Mid( Count - count, count );
	}

	template <typename T>
	inline Span<const T> Array<T>::Right( int count ) const
	{
		return Mid( Count - count, count );
	}

	template <typename T>
	inline Span<T> Array<T>::Mid( int index, int count )
	{
		NeAssertBounds(index, Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		NeAssert((index + count) <= Count);
		return Span<T> { Item + index, count };
	}

	template <typename T>
	inline Span<const T> Array<T>::Mid( int index, int count ) const
	{
		NeAssertBounds(index, Count);
		NeAssertOut(count >= 0, "Invalid count: %d", count);
		NeAssert((index + count) <= Count);
		return Span<const T> { Item + index, count };
	}

	//==================================================================================

	template <typename T>
	inline T* begin( Array<T>& a )
	{
		return a.Item;
	}

	template <typename T>
	inline const T* begin( const Array<T>& a )
	{
		return a.Item;
	}

	template <typename T>
	inline T* end( Array<T>& a )
	{
		return a.Item + a.Count;
	}

	template <typename T>
	inline const T* end( const Array<T>& a )
	{
		return a.Item + a.Count;
	}

	//==================================================================================

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
}

