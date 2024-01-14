#pragma once

//======================================================================================
#include "Span.h"
#include "Memory.h"

//======================================================================================
namespace Nemesis
{
	struct ReBase
	{
		enum Op
		{ Attach
		, Detach
		};
	};

	struct TableSetup
	{
		enum Mode
		{ CopyData
		, CalcSize
		};
	};

	template < typename T >
	struct Table
	{
	public:
		union
		{
			T*		 Item;
			uint8_t* Data;
			int64_t	 Offset;
		};
		int32_t Count;
		int32_t User;

	public:
		T&		 operator [] ( int index );
		const T& operator [] ( int index ) const;

	public:
		operator Span<T>		();
		operator Span<const T>	() const;
	};

	//==================================================================================

	template <typename T>
	inline T& Table<T>::operator [] ( int index )	
	{ 
		NeAssertBounds(index, Count);
		return Item[index]; 
	}

	template <typename T>
	inline const T& Table<T>::operator [] ( int index ) const 
	{ 
		NeAssertBounds(index, Count); 
		return Item[index]; 
	}

	template <typename T>
	inline Table<T>::operator Span<T>()
	{
		return Span<T> { Item, Count };
	}

	template <typename T>
	inline Table<T>::operator Span<const T>() const
	{
		return Span<const T> { Item, Count };
	}

	//==================================================================================

	template < typename T >
	inline void Table_Init( Table<T>* table, int count )
	{ 
		table->Item = nullptr;
		table->Count = count; 
		table->User = 0;
	}

	template < typename T >
	inline uint8_t* Table_Bind( Table<T>* table, uint8_t* data )
	{ 
		table->Data = table->Count ? data : nullptr;
		return data + Mem_Align_16( sizeof(T) * table->Count );
	}

	template < typename T >
	inline T* Table_Copy( Table<T>* table, const T* item, int count )
	{
		NeAssert( table->Count == count );
		return Arr_Cpy( table->Item, item, count );
	}

	template < typename T >
	inline void Table_ReBase( Table<T>* table, ReBase::Op op )
	{
		switch (op)
		{
		case ReBase::Attach:
			NeAssertOut(!table->Data || (Ptr_Tell( table, table->Data ) < 0), "Trying to attach a table that is already located.");
			if (table->Data)
				table->Data += (ptrdiff_t)table;
			NeAssert((table->Offset % 16) == 0);
			break;

		case ReBase::Detach:
			NeAssertOut(!table->Data || (Ptr_Tell( table, table->Data ) > 0), "Trying to detach a table that is already dislocated.");
			if (table->Data)
				table->Data -= (ptrdiff_t)table;
			NeAssert((table->Offset % 16) == 0);
			break;

		default:
			NeAssertOut(false, "Invalid re-base operation: %d.", op );
			break;
		}
	}

	//==================================================================================

	template < typename T >
	inline uint8_t* Table_Setup( Table<T>* table, uint8_t* pos, const T* item, int count, TableSetup::Mode mode )
	{ 
		Table_Init( table, count );
		pos = Table_Bind( table, pos );
		if (mode == TableSetup::CalcSize)
			return pos;
		Table_Copy( table, item, count );
		return pos;
	}

	template < typename T >
	inline uint8_t* Table_Setup( Table<T>* table, uint8_t* pos, const Span<const T>& items, TableSetup::Mode mode )
	{
		return Table_Setup( table, pos, items.Item, items.Count, mode );
	}

	template < typename T >
	inline uint8_t* Table_Setup( Table<T>* table, uint8_t* pos, const Span<T>& items, TableSetup::Mode mode )
	{
		return Table_Setup( table, pos, items.Item, items.Count, mode );
	}

	//==================================================================================

	template <typename T>
	inline T* begin( Table<T>& a )
	{
		return a.Item;
	}

	template <typename T>
	inline const T* begin( const Table<T>& a )
	{
		return a.Item;
	}

	template <typename T>
	inline T* end( Table<T>& a )
	{
		return a.Item + a.Count;
	}

	template <typename T>
	const T* end( const Table<T>& a )
	{
		return a.Item + a.Count;
	}

}

//======================================================================================
//	Name Table
//======================================================================================
namespace Nemesis
{
	struct NameTable_s
	{
		Table<int32_t> Offsets;
		Table<char>	   Strings;
	};

	//==================================================================================

	uint8_t* NE_API NameTable_Bind		 ( NameTable_s* table, uint8_t* data );
	void	 NE_API NameTable_ReBase	 ( NameTable_s* table, ReBase::Op op );
	void	 NE_API NameTable_Init		 ( NameTable_s* table, const cstr_t* item, int count );
	void	 NE_API NameTable_CopyData	 ( NameTable_s* table, const cstr_t* item, int count );
	uint8_t* NE_API NameTable_Setup		 ( NameTable_s* table, uint8_t* pos, const cstr_t* item, int count, TableSetup::Mode mode );
	int		 NE_API NameTable_GetNumNames( const NameTable_s* table );
	cstr_t	 NE_API NameTable_GetName	 ( const NameTable_s* table, int index );

	//==================================================================================

	inline uint8_t* NameTable_Setup( NameTable_s* table, uint8_t* pos, const Span<const cstr_t>& items, TableSetup::Mode mode )
	{
		return NameTable_Setup( table, pos, items.Item, items.Count, mode );
	}
}
