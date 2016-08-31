//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform.h: Operating system specific includes and defines.

#ifndef COMMON_PLATFORM_H_
#define COMMON_PLATFORM_H_

#if defined(_WIN32) || defined(_WIN64)
#   define ANGLE_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
#   define ANGLE_PLATFORM_APPLE 1
#   define ANGLE_PLATFORM_POSIX 1
#elif defined(ANDROID)
#   define ANGLE_PLATFORM_ANDROID 1
#   define ANGLE_PLATFORM_POSIX 1
#elif defined(__linux__) || defined(EMSCRIPTEN)
#   define ANGLE_PLATFORM_LINUX 1
#   define ANGLE_PLATFORM_POSIX 1
#elif defined(__FreeBSD__) || \
      defined(__OpenBSD__) || \
      defined(__NetBSD__) || \
      defined(__DragonFly__) || \
      defined(__sun) || \
      defined(__GLIBC__) || \
      defined(__GNU__) || \
      defined(__QNX__)
#   define ANGLE_PLATFORM_POSIX 1
#else
#   error Unsupported platform.
#endif

#ifdef ANGLE_PLATFORM_WINDOWS
#   ifndef STRICT
#       define STRICT 1
#   endif
#   ifndef WIN32_LEAN_AND_MEAN
#       define WIN32_LEAN_AND_MEAN 1
#   endif
#   ifndef NOMINMAX
#       define NOMINMAX 1
#   endif

#   include <windows.h>
#   include <intrin.h>

#   if defined(WINAPI_FAMILY) && (WINAPI_FAMILY != WINAPI_FAMILY_DESKTOP_APP)
#       define ANGLE_ENABLE_WINDOWS_STORE 1
#   endif

// This enables the Windows Holographic code path.
// Comment this out to revert back to non-Windows Holographic functionality in most places. This is
// designed to ensure that real-time functionality can be reverted, if necessary, for testing and/or
// for non-holographic UWP builds of multi-target applications.
// To build with this enabled, use winrt\10\src\angle.sln - which includes all the source code files
// that are specific to Windows Holographic.
#define ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC

#   if defined(ANGLE_ENABLE_D3D9)
#       include <d3d9.h>
#       include <d3dcompiler.h>
#   endif

#   if defined(ANGLE_ENABLE_D3D11)
#       include <d3d10_1.h>
#       include <d3d11.h>
#       if defined(ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC)
#           include <d3d11_4.h>
#       else
#           include <d3d11_1.h>
#       endif
#       include <dxgi.h>
#       include <dxgi1_2.h>
#       include <d3dcompiler.h>
#   endif

#   if defined(ANGLE_ENABLE_WINDOWS_STORE)
#       if defined(ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC)
#           include <dxgi1_4.h>
#       else
#           include <dxgi1_3.h>
#       endif
#       if defined(_DEBUG)
#           include <DXProgrammableCapture.h>
#           include <dxgidebug.h>
#       endif
#   endif

#   undef near
#   undef far
#endif

#if !defined(_M_ARM) && !defined(ANGLE_PLATFORM_ANDROID)
#   define ANGLE_USE_SSE
#endif

#endif // COMMON_PLATFORM_H_
