/* FILE NAME   : 'def.h'
 * PURPOSE     : Global definitions and includes header file.
 * PROGRAMMER  : Fedor Borodulin.
 * LAST UPDATE : 10.04.2023.
 */

#ifndef __def_h__
#define __def_h__

#define _CRT_SECURE_NO_WARNINGS

/* Win32 API headers */
#include <Windows.h>
#include <Windowsx.h>
#include "../res/resource.h"

/* Smart pointers for COM */
#include <wrl.h>
using Microsoft::WRL::ComPtr;

/* I/O headers */
#include <cstdio>
#include <iostream>
#include <fstream>

/* Math headers */
#define _USE_MATH_DEFINES
#include <cmath>
#include <intrin.h>
#include <xmmintrin.h>
#include <immintrin.h>

/* Types headers */
#include <type_traits>
#include <limits>

/* Data structures headers */
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <string>

/* Auxilary functional headers */
#include <algorithm>
#include <functional>
#include <thread>
#include <mutex>
#include <filesystem>

/* Undefine annoying standard macro-functions */
#undef min
#undef max

/* Shorter definitions for float integer types */
using flt = float;
using dbl = double;

/* Coordinates storage structures */
struct coordf { flt X, Y; };
struct coordd { dbl X, Y; };

#endif /* __def_h__ */

/* END OF 'def.h' FILE */
