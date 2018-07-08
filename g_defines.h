#ifndef G_DEFINES_H
#define G_DEFINES_H

template<typename T>
inline constexpr T x64x32(T x64, T x32)
{
	if (sizeof(void*) == 0x8)
		return x64;
	return x32;
}

#define COMMA ,

#if defined(_WIN32)
#define SECTION(sec) __declspec(allocate(sec))
#define WSECTION(sec) SECTION(sec)
#include "wincludes.h"
#include <intrin.h>
#include <string.h>
#define OLin(Linux)
#define OWin(Windows) Windows
#define PosixWin(Posix, Windows) Windows
#define LWM(Linux, Windows, Mac) Windows
#define OMac(Mac)
#define OPosix(Posix)
#define paddr(handle, name) GetProcAddress(handle, name)
#define FASTARGS void* thisptr, void* edx
#define CFASTARGS thisptr, edx
#define STDARGS
#define THISARGS void* thisptr
#define LC
#define PC
#define WC COMMA
#define _noinline __declspec(noinline)
#elif defined(__linux__)
#define __posix__
#define SECTION(sec) __attribute__((section(sec)))
#define WSECTION(sec)
#define OLin(Linux) Linux
#define OWin(Windows)
#define OMac(Mac)
#define OPosix(Posix) Posix
#define PosixWin(Posix, Windows) Posix
#define LWM(Linux, Windows, Mac) Linux
#define paddr(handle, name) dlsym(handle, name)
#define FASTARGS void* thisptr
#define CFASTARGS thisptr
#define STDARGS void* thisptr
#define THISARGS void* thisptr
#define LC COMMA
#define PC COMMA
#define WC
#define _ReturnAddress() __builtin_return_address(0)
#define _noinline __attribute__((noinline))
#else
#define __posix__
#define SECTION(sec) __attribute__((section(sec)))
#define WSECTION(sec)
#define OLin(Linux)
#define OWin(Windows)
#define OMac(Mac) Mac
#define OPosix(Posix) Posix
#define PosixWin(Posix, Windows) Posix
#define LWM(Linux, Windows, Mac) Mac
#define paddr(handle, name) dlsym(handle, name)
#define FASTARGS void* thisptr
#define CFASTARGS thisptr
#define STDARGS void* thisptr
#define THISARGS void* thisptr
#define LC COMMA
#define PC COMMA
#define WC
#define _ReturnAddress() __builtin_return_address(0)
#define _noinline __attribute__((noinline))
#endif

#ifdef __posix__
#define __thiscall
#define __fastcall
#define __stdcall
#define __cdecl
#define __declspec (a)
#define _stricmp(a, b) strcasecmp(a, b)
#endif

#ifdef _MSC_VER
#define FRAME_POINTER() *((void**)_AddressOfReturnAddress() - 1)
#else
#define FRAME_POINTER() __builtin_frame_address(0)
#endif

#endif
