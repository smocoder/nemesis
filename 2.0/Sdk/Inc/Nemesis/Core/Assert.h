#pragma once

//======================================================================================
#include "Types.h"

//======================================================================================
//	Assert Types
//======================================================================================
namespace Nemesis
{
	struct AssertAction
	{
		enum Kind
		{ Continue
		, Terminate
		, Break
		};
	};

	struct AssertInfo_s
	{
		cstr_t Expression;
		cstr_t Function;
		cstr_t File;
		int32_t Line;
	};

	struct AssertHook_s
	{
		AssertAction::Kind (NE_CALLBK *AssertFailed)( ptr_t context, const AssertInfo_s& info, cstr_t msg );
		ptr_t Context;
	};

}

//======================================================================================
//	Assert Api
//======================================================================================
namespace Nemesis
{
	AssertHook_s		NE_API Assert_GetHook();
	void				NE_API Assert_SetHook( const AssertHook_s& hook );
	AssertAction::Kind	NE_API Assert_Failure( const AssertInfo_s& info, cstr_t msg, ... );
}

//======================================================================================
//	Assert Macros
//======================================================================================
#define NeAssertBreak() \
	NeDebugBreak()

#define NeAssertFailed(expr, ...) \
	( \
		(::Nemesis::Assert_Failure( ::Nemesis::AssertInfo_s { #expr, __FUNCTION__, __FILE__, __LINE__  }, __VA_ARGS__ )) \
			== ::Nemesis::AssertAction::Break \
			? (void)NeAssertBreak() \
			: (void)(0) \
	)

#define NeAssertEval(expr, ...) \
	( \
		(void)												/* strip off the result since we don't use it (clang will emit a warning)*/ \
			(	(expr)										/* either the expression evaluates to 'true' and the assertion holds */ \
			||  (											/* or the assetion failed */ \
				  NeAssertFailed(expr,__VA_ARGS__)			/*		and we call the 'failed assertion handler' */ \
				, false										/*		while evaluating to a boolean */ \
				) \
			) \
	)

#define NeAssertOutImpl(expr, ...) \
	do \
	{ \
		NeAssertEval(expr, __VA_ARGS__); \
		/*__analysis_assume(expr);*/ \
	} \
	while(false)

#define NeAssertOutOnceImpl(expr, ...) \
	do \
	{ \
		static bool once = true; \
		if ( once ) \
		{ \
			const bool result = NeAssertEval(expr, __VA_ARGS__); \
			once &= result; \
		} \
		/*__analysis_assume(expr);*/ \
	} \
	while(false)

//======================================================================================
#if !defined NE_ENABLE_ASSERT
#	define	 NE_ENABLE_ASSERT (NE_CONFIG < NE_CONFIG_MASTER)
#endif

//======================================================================================
#if NE_ENABLE_ASSERT
#	define NeAssert(expr)				NeAssertOutImpl(expr, "")
#	define NeAssertOut(expr, ...)		NeAssertOutImpl(expr, __VA_ARGS__)
#	define NeAssertOutOnce(expr, ...)	NeAssertOutOnceImpl(expr, __VA_ARGS__)
#else
#	define NeAssert(expr)				//__noop(expr)
#	define NeAssertOut(expr, ...)		//__noop(expr, __VA_ARGS__)
#	define NeAssertOutOnce(expr, ...)	//__noop(expr, __VA_ARGS__)
#endif

//======================================================================================
#define NeAssertBounds( i, n ) NeAssertOut( (i >= 0) && (i < n), "Index out of bounds: %d. Count: %d.", i, n )
