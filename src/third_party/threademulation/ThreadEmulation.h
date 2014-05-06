// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//
// Emulates a subset of the Win32 threading API as a layer on top of WinRT threadpools.
//
// Supported features:
//
//    - CreateThread (returns a standard Win32 handle which can be waited on, then closed)
//    - CREATE_SUSPENDED and ResumeThread
//    - Partial support for SetThreadPriority (see below)
//    - Sleep
//    - Thread local storage (TlsAlloc, TlsFree, TlsGetValue, TlsSetValue)
//
// Differences from Win32:
//
//    - If using TLS other than from this CreateThread emulation, call TlsShutdown before thread/task exit
//    - No ExitThread or TerminateThread (just return from the thread function to exit)
//    - No SuspendThread, so ResumeThread is only useful in combination with CREATE_SUSPENDED
//    - SetThreadPriority is only available while a thread is in CREATE_SUSPENDED state
//    - SetThreadPriority only supports three priority levels (negative, zero, or positive)
//    - No thread identifier APIs (GetThreadId, GetCurrentThreadId, OpenThread)
//    - No affinity APIs
//    - No GetExitCodeThread
//    - Failure cases return error codes but do not always call SetLastError

#pragma once

#include <winapifamily.h>
#if defined(WINAPI_FAMILY)
#if !WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)

#include <stdlib.h>

#define TLS_OUT_OF_INDEXES -1

#ifndef CREATE_SUSPENDED
#define CREATE_SUSPENDED 0x00000004
#endif

namespace ThreadEmulation
{
    void Sleep(_In_ unsigned long dwMilliseconds);

    unsigned long TlsAlloc();
    int TlsFree(_In_ unsigned long dwTlsIndex);
    void* TlsGetValue(_In_ unsigned long dwTlsIndex);
    int TlsSetValue(_In_ unsigned long dwTlsIndex, _In_opt_ void* lpTlsValue);

    void TlsShutdown();

    inline void * LocalAlloc(unsigned int uFlags, size_t size) { return malloc(size); };
    inline void LocalFree(void* index) { free((void*)index); };
}

#endif
#endif