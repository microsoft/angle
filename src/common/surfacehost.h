//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// surfacehost.h: Defines SurfaceHost, a class for managing and performing
// operations on an EGLNativeWindowType. It is used for HWND (Desktop Windows)
// and IInspectable objects (Windows Store Applications).

#ifndef COMMON_SURFACEHOST_H_
#define COMMON_SURFACEHOST_H_

#include <EGL/eglplatform.h>
#include "common/debug.h"
#include <dxgi.h>
#include <dxgi1_2.h>
#if defined (ANGLE_ENABLE_WINDOWS_STORE)
#include <dxgi1_3.h>
#endif // defined (ANGLE_ENABLE_WINDOWS_STORE)

#include <d3d11.h>

#if defined(ANGLE_ENABLE_WINDOWS_STORE)
typedef IDXGISwapChain1 DXGISwapChain;
typedef IDXGIFactory2 DXGIFactory;

#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.applicationmodel.core.h>
#include <memory>

class IInspectableSurfaceHost;

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

#else
typedef IDXGISwapChain DXGISwapChain;
typedef IDXGIFactory DXGIFactory;
#endif

namespace rx
{
class SurfaceHost
{
public:
    SurfaceHost(EGLNativeWindowType window);
    ~SurfaceHost();

#if !defined(ANGLE_ENABLE_WINDOWS_STORE)
    // The HWND SurfaceHost implementation can benefit
    // by having inline versions of these methods to 
    // reduce the calling overhead.
    inline bool initialize() { return true; }
    inline bool getClientRect(LPRECT rect) { return !!GetClientRect(mWindow, rect); }
    inline bool isIconic() { return !!IsIconic(mWindow); }
#else
    bool initialize();
    bool getClientRect(LPRECT rect);
    bool isIconic();
#endif // !defined(ANGLE_ENABLE_WINDOWS_STORE)

    HRESULT createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, UINT width, UINT height, DXGISwapChain** swapChain);
    inline EGLNativeWindowType getNativeWindowType() const { return mWindow; }

private:
    EGLNativeWindowType mWindow;

#if defined(ANGLE_ENABLE_WINDOWS_STORE)
    std::shared_ptr<IInspectableSurfaceHost> mImpl;
#endif // defined(ANGLE_ENABLE_WINDOWS_STORE)

};
}

bool isValid(EGLNativeWindowType window);

#endif // COMMON_SURFACEHOST_H_
