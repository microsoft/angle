//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// iinspectablehost.h: Host specific implementation interface for 
// managing IInspectable native window types.

#ifndef COMMON_IINSPECTABLEHOST_H_
#define COMMON_IINSPECTABLEHOST_H_

#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.applicationmodel.core.h>
#include <windows.ui.xaml.h>
#include <windows.ui.xaml.media.dxinterop.h>
#include "common/winrt/winrtutils.h"
#include "common/surfaceHost.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;

class IInspectableSurfaceHost
{
public:
    virtual bool initialize(EGLNativeWindowType window) = 0;
    virtual HRESULT createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, unsigned int width, unsigned int height, DXGISwapChain** swapChain) = 0;
    virtual bool registerForSizeChangeEvents() = 0;
    virtual void unregisterForSizeChangeEvents() = 0;

    IInspectableSurfaceHost() :
        mSupportsSwapChainResize(true),
        mClientRectChanged(false),
        mClientRect({0,0,0,0}),
        mNewClientRect({0,0,0,0})
    {
        mSizeChangedEventToken.value = 0;
    }

    virtual ~IInspectableSurfaceHost(){}

    bool getClientRect(RECT* rect)
    {
        if (mClientRectChanged && mSupportsSwapChainResize)
        {
            mClientRect = mNewClientRect;
        }

        *rect = mClientRect;

        return true;
    }

    void setNewClientSize(SIZE& newSize)
    {
        if (mSupportsSwapChainResize)
        {
            mNewClientRect = { 0, 0, newSize.cx, newSize.cy };
            mClientRectChanged = true;
        }
    }

protected:
    bool mSupportsSwapChainResize;
    RECT mClientRect;
    RECT mNewClientRect;
    bool mClientRectChanged;

    EventRegistrationToken mSizeChangedEventToken;
};

bool isCoreWindow(EGLNativeWindowType window, ComPtr<ABI::Windows::UI::Core::ICoreWindow>* coreWindow = nullptr);
bool isSwapChainPanel(EGLNativeWindowType window, ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel>* swapChainPanel = nullptr);

#endif // COMMON_IINSPECTABLEHOST_H_
