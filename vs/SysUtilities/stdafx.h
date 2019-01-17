// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers

#include <stdlib.h>
#include <stdio.h>
// Windows Header Files:
#include <windows.h>


#include <functional>
#include <list>
// TODO: reference additional headers your program requires here
#include "../include/SysUtilities.h"
#include "../include/Debug.h"
#include "../src/InDef.h"
#include "../include/api.h"

using namespace SYS_UTL;

#pragma warning(disable: 4251)