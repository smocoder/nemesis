#pragma once

//======================================================================================
//	Windows
//======================================================================================
#include <SDKDDKVer.h>
#include <windows.h>

//======================================================================================
//	C Runtime
//======================================================================================
#include <stdio.h>
#include <tchar.h>

//======================================================================================
// Nemesis
//======================================================================================
#include <Nemesis/Core/Types.h>
#include <Nemesis/Core/Assert.h>
#include <Nemesis/Core/Debug.h>
#include <Nemesis/Core/Memory.h>
#include <Nemesis/Core/Array.h>
#include <Nemesis/Core/Search.h>
#include <Nemesis/Core/Table.h>
#include <Nemesis/Core/String.h>
NE_LINK("libNeCore.lib")
