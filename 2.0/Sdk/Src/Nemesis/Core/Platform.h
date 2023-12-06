#pragma once

//======================================================================================
#include <Nemesis/Core/Types.h>

//======================================================================================
#if ((NE_PLATFORM == NE_PLATFORM_WIN32) || (NE_PLATFORM == NE_PLATFORM_WIN64))
#
#	ifndef _WIN32_WINNT
#		define _WIN32_WINNT 0x0600
#		define WIN32_LEAN_AND_MEAN
#		include <windows.h>
#	endif
#
#elif (NE_PLATFORM == NE_PLATFORM_DURANGO)
#
#	include <windows.h>
#
#elif (NE_PLATFORM == NE_PLATFORM_SCARLETT)
#
#	include <windows.h>
#
#elif (NE_PLATFORM == NE_PLATFORM_PS4)
#
#	include <sdk_version.h>
#	include <sce_atomic.h>
#	include <kernel.h>
#
#elif (NE_PLATFORM == NE_PLATFORM_PS5)
#
#	include <sdk_version.h>
#	include <sce_atomic.h>
#	include <kernel.h>
#
#else
#
#	error Platform not supported...
#
#endif
