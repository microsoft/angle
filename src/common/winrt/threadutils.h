//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angleutils.h: Common ANGLE utilities.

#ifndef COMMON_WINRT_THREADUTILS_H_
#define COMMON_WINRT_THREADUTILS_H_

#include <windows.h>

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE)
//#include <processthreadsapi.h>

namespace ThreadUtilsWinRT
{


#ifdef __cplusplus
extern "C" {
#endif

#define TLS_OUT_OF_INDEXES ((DWORD)0xFFFFFFFF)

_Must_inspect_result_
WINBASEAPI
DWORD
WINAPI
TlsAlloc(
    VOID
    );


WINBASEAPI
LPVOID
WINAPI
TlsGetValue(
    _In_ DWORD dwTlsIndex
    );


WINBASEAPI
BOOL
WINAPI
TlsSetValue(
    _In_ DWORD dwTlsIndex,
    _In_opt_ LPVOID lpTlsValue
    );


WINBASEAPI
BOOL
WINAPI
TlsFree(
    _In_ DWORD dwTlsIndex
    );

inline void LocalFree(HLOCAL index)
{
    free((void*) index);
}

inline void* LocalAlloc(UINT uFlags, size_t size)
{
    return malloc(size);
}

#ifdef __cplusplus
}
#endif

}


#elif WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_APP)

namespace ThreadUtilsWinRT
{

inline DWORD TlsAlloc(){
	return FlsAlloc(nullptr);
}

inline void LocalFree(HLOCAL index)
{
    free((void*) index);
}

inline void* LocalAlloc(UINT uFlags, size_t size)
{
    return malloc(size);
}

#define TlsFree FlsFree


#define TLS_OUT_OF_INDEXES FLS_OUT_OF_INDEXES
#define TlsSetValue FlsSetValue
#define TlsGetValue FlsGetValue

}

#endif

#endif // COMMON_WINRT_THREADUTILS_H_
