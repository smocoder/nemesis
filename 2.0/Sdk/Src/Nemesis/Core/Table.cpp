#include "stdafx.h"
#include <Nemesis/Core/Table.h>
#include <Nemesis/Core/Assert.h>
#include <Nemesis/Core/Debug.h>
#include <Nemesis/Core/String.h>

//======================================================================================
//	Private
//======================================================================================
namespace Nemesis
{
	static int StrTable_Size( const cstr_t* item, int count )
	{
		if (!item || !count)
			return 0;
		int sum = 0;
		for ( int i = 0; i < count; ++i )
			sum += item[i] ? (1+Str_Len( item[i] )) : 0;
		return sum;
	}
}

//======================================================================================
//	Name Table
//======================================================================================
namespace Nemesis
{
	uint8_t* NameTable_Bind( NameTable_s* table, uint8_t* data )
	{
		data = Table_Bind( &table->Offsets, data );
		data = Table_Bind( &table->Strings, data );
		return data;
	}

	void NameTable_ReBase( NameTable_s* table, ReBase::Op op )
	{
		Table_ReBase( &table->Offsets, op );
		Table_ReBase( &table->Strings, op );
	}

	void NameTable_Init( NameTable_s* table, const cstr_t* item, int count )
	{
		const int32_t str_size = StrTable_Size( item, count );

		Table_Init( &table->Offsets, str_size ? count					 : 0 );
		Table_Init( &table->Strings, str_size ? Mem_Align_16( str_size ) : 0 );
	}

	void NameTable_CopyData( NameTable_s* table, const cstr_t* item, int count )
	{
		if (!table->Offsets.Count)
			return;

		NeAssert(table->Offsets.Count == count);

		int len;
		char* begin = table->Strings.Item;
		char* pos = begin;
		for ( int i = 0; i < table->Offsets.Count; ++i )
		{
			// offset
			table->Offsets.Item[i] = (int)(pos-begin);

			// string
			len = item[i] ? 1+Str_Len( item[i] ) : 0;
			Mem_Cpy( pos, item[i], len );

			// next
			pos = pos + len;
		}
	}

	uint8_t* NameTable_Setup( NameTable_s* table, uint8_t* pos, const cstr_t* item, int count, TableSetup::Mode mode )
	{
		NameTable_Init( table, item, count );
		pos = NameTable_Bind( table, pos );
		if (mode == TableSetup::CalcSize)
			return pos;
		NameTable_CopyData( table, item, count );
		return pos;
	}

	int NameTable_GetNumNames( const NameTable_s* table )
	{
		return table->Offsets.Count;
	}

	cstr_t NameTable_GetName( const NameTable_s* table, int index )
	{
		if (!table->Offsets.Data)
			return nullptr;
		if ((index < 0) || (index >= table->Offsets.Count))
			return nullptr;
		return table->Strings.Item + table->Offsets.Item[ index ];
	}
}
