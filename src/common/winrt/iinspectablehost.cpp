//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// iinspectablehost.cpp: Host for managing IInspectable native window types.

#include "common/winrt/corewindowhost.h"

namespace rx
{
SurfaceHost::SurfaceHost(EGLNativeWindowType window)
{
    mWindow = window;
}

SurfaceHost::~SurfaceHost()
{

}

bool SurfaceHost::initialize()
{
    try
    {
        ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow;
        if (isCoreWindow(mWindow, &coreWindow))
        {
            mImpl = std::make_shared<CoreWindowHost>();
            if (mImpl)
            {
                return mImpl->initialize(mWindow);
            }
        }
    }
    catch (std::bad_alloc)
    {
        return false;
    }

    return false;
}

bool SurfaceHost::getClientRect(RECT* rect)
{
    if (mImpl)
    {
        return mImpl->getClientRect(rect);
    }

    return false;
}

bool SurfaceHost::isIconic()
{
    return false;
}

HRESULT SurfaceHost::createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, unsigned int width, unsigned int height, DXGISwapChain** swapChain)
{
    if (mImpl)
    {
        return mImpl->createSwapChain(device, factory, format, width, height, swapChain);
    }

    return E_UNEXPECTED;
}

}

bool isCoreWindow(EGLNativeWindowType window, ComPtr<ABI::Windows::UI::Core::ICoreWindow>* coreWindow)
{
    if (!window)
    {
        return false;
    }

    ComPtr<IInspectable> win = window;
    ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWin;
    if (SUCCEEDED(win.As(&coreWin)))
    {
        if (coreWindow != nullptr)
        {
            *coreWindow = coreWin.Detach();
        }
        return true;
    }

    return false;
}

bool isValid(EGLNativeWindowType window)
{
    return isCoreWindow(window);
}