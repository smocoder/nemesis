#pragma once

//======================================================================================
#include "Array.h"

//======================================================================================
namespace Nemesis
{
	struct Page_s
	{
		uint8_t* Data;
		uint8_t* End;
		uint8_t* Pos;

		size_t Used()     const { return Pos-Data; }
		size_t Remain()   const { return End-Pos;  }
		size_t Capacity() const { return End-Data; }

		void Reset() { Pos = Data; }
	};

	struct Stack_s
	{
		enum 
		{ MIN_PAGE_SIZE		= 1024
		, DEFAULT_PAGE_SIZE	= 4096
		, DEFAULT_ALIGN		= 8
		};


	public:
		explicit Stack_s( Alloc_t alloc );
		Stack_s();
		~Stack_s();

	public:
		void Init( Alloc_t alloc, size_t page_size );
		void Init( Alloc_t alloc );
		void Clear();
		void Reset();

	public:
		void*	Alloc		( size_t size, size_t align );
		void*	Alloc		( size_t size );
		Page_s* FindPage	( size_t size ) const;
		Page_s* EnsurePage	( size_t size );
		Page_s* AppendPage	( size_t size );

	public:
		size_t Align;
		size_t PageSize;
		Array<Page_s*> Pages;
	};

	//==================================================================================

	inline Stack_s::Stack_s( Alloc_t alloc )
		: Align(0)
		, PageSize(0)
		, Pages(alloc)
	{}

	inline Stack_s::Stack_s()
		: Align(0) 
		, PageSize(0)
	{}

	inline Stack_s::~Stack_s()
	{ 
		Clear(); 
	}

	inline void Stack_s::Init( Alloc_t alloc, size_t page_size )
	{
		Pages.Alloc = alloc;
		PageSize = page_size;
	}

	inline void Stack_s::Init( Alloc_t alloc )
	{
		Pages.Alloc = alloc;
	}

	inline void Stack_s::Clear()
	{
		for ( int i = 0; i < Pages.Count; ++i )
			Mem_Free( Pages.Alloc, Pages[i] );
		Array_Clear(Pages);
	}

	inline void Stack_s::Reset()
	{
		for ( int i = 0; i < Pages.Count; ++i )
			Pages[i]->Reset();
	}

	inline void* Stack_s::Alloc( size_t size, size_t align )
	{
		const size_t full_align = align ? align : DEFAULT_ALIGN;
		NeAssert(Size_IsPow2(full_align));

		const size_t full_size = size + full_align;
		Page_s* page = EnsurePage( full_size );

		page->Pos = (uint8_t*)PtrDiff_Align_Pow2( (ptrdiff_t)page->Pos, full_align );
		NeAssert((page->End - page->Pos) >= (ptrdiff_t)size);

		void* item = page->Pos;
		page->Pos += size;

		NeAssert(page->Pos < page->End);
		NeAssert(Ptr_IsAligned_Pow2(item, full_align));
		return item;
	}

	inline void* Stack_s::Alloc( size_t size )
	{
		return Alloc( size, Align );
	}

	inline Page_s* Stack_s::FindPage( size_t size ) const
	{
		for ( int i = Pages.Count-1; i >= 0; --i )
			if ( Pages[i]->Remain() >= size )
				return Pages[i];
		return nullptr;
	}

	inline Page_s* Stack_s::EnsurePage( size_t size )
	{
		Page_s* page = FindPage( size );
		if (page)
			return page;

		size_t page_size = PageSize ? PageSize : DEFAULT_PAGE_SIZE;
		if (page_size < MIN_PAGE_SIZE)
			page_size = MIN_PAGE_SIZE;

		if (size > page_size)
			page_size = Size_Align_Pow2(size, MIN_PAGE_SIZE);

		return AppendPage( page_size );
	}

	inline Page_s* Stack_s::AppendPage( size_t size )
	{
		uint8_t* page_buffer = (uint8_t*)Mem_Alloc( Pages.Alloc, size );
		Page_s* page = (Page_s*)page_buffer;

		page->Data	= (page_buffer + sizeof(Page_s));
		page->Pos	= (page_buffer + sizeof(Page_s));
		page->End	= (page_buffer + size);

		NeAssertOut(Pages.Count != 64, "Stack exceeding 64 pages! Please consider increasing the page size!");
		Array_Append(Pages, page);
		return page;
	}

	//==================================================================================

	template  < typename T >
	inline T* Stack_Alloc( Stack_s& stack ) 
	{
		return (T*)(stack.Alloc( sizeof(T) ));
	}

	template  < typename T >
	inline T* Stack_Alloc( Stack_s& stack, int count ) 
	{
		return (T*)(stack.Alloc( count * sizeof(T) ));
	}

	template  < typename T >
	inline T* Stack_Calloc( Stack_s& stack ) 
	{
		return (T*)(Mem_Zero( stack.Alloc( sizeof(T) ), sizeof(T) ));
	}

	template  < typename T >
	inline T* Stack_Calloc( Stack_s& stack, int count ) 
	{
		return (T*)(Mem_Zero( stack.Alloc( count * sizeof(T) ), count * sizeof(T) ));
	}

}
