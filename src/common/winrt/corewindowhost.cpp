//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// corewindowhost.cpp: Host for managing ICoreWindow native window types.

#include "common/winrt/corewindowhost.h"

CoreWindowHost::~CoreWindowHost()
{
    unregisterForSizeChangeEvents();
}

bool CoreWindowHost::initialize(EGLNativeWindowType window)
{
    ComPtr<IInspectable> win = window;
    if (SUCCEEDED(win.As(&mCoreWindow)))
    {
        if (SUCCEEDED(getCoreWindowSizeInPixels(mCoreWindow, &mClientRect)))
        {
            mNewClientRect = mClientRect;
            mClientRectChanged = false;
            return registerForSizeChangeEvents();
        }
        else
        {
            return false;
        }
    }
    return false;
}

bool CoreWindowHost::registerForSizeChangeEvents()
{
    ComPtr<IWindowSizeChangedEventHandler> sizeChangedHandler;
    if (SUCCEEDED(Microsoft::WRL::MakeAndInitialize<CoreWindowSizeChangedHandler>(sizeChangedHandler.ReleaseAndGetAddressOf(), this->shared_from_this())))
    {
        if (SUCCEEDED(mCoreWindow->add_SizeChanged(sizeChangedHandler.Get(), &mSizeChangedEventToken)))
        {
            return true;
        }
    }

    return false;
}

void CoreWindowHost::unregisterForSizeChangeEvents()
{
    (void)mCoreWindow->remove_SizeChanged(mSizeChangedEventToken);
    mSizeChangedEventToken.value = 0;
}

HRESULT CoreWindowHost::createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, unsigned int width, unsigned int height, DXGISwapChain** swapChain)
{
    HRESULT result = S_OK;
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = format;
    swapChainDesc.Stereo = FALSE;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;

    ComPtr<IDXGISwapChain1> newSwapChain;
    result = factory->CreateSwapChainForCoreWindow(device, mCoreWindow.Get(), &swapChainDesc, nullptr, newSwapChain.ReleaseAndGetAddressOf());
    if (SUCCEEDED(result))
    {

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
        // Test if swapchain supports resize.  On Windows Phone devices, this will return DXGI_ERROR_UNSUPPORTED.  On
        // other devices DXGI_ERROR_INVALID_CALL should be returned because the combination of flags passed
        // (DXGI_SWAP_CHAIN_FLAG_NONPREROTATED | DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE) are invalid flag combinations.
        if (newSwapChain->ResizeBuffers(swapChainDesc.BufferCount, swapChainDesc.Width, swapChainDesc.Height, swapChainDesc.Format, DXGI_SWAP_CHAIN_FLAG_NONPREROTATED | DXGI_SWAP_CHAIN_FLAG_GDI_COMPATIBLE) == DXGI_ERROR_UNSUPPORTED)
        {
            mSupportsSwapChainResize = false;
        }
#endif // (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)

        if (mSupportsSwapChainResize == false)
        {
            unregisterForSizeChangeEvents();
        }

        newSwapChain.CopyTo(swapChain);
    }

    return result;
}

HRESULT getCoreWindowSizeInPixels(ComPtr<ABI::Windows::UI::Core::ICoreWindow> coreWindow, RECT* windowSize)
{
    HRESULT result = S_OK;

    ABI::Windows::Foundation::Rect bounds;
    result = coreWindow->get_Bounds(&bounds);
    if (SUCCEEDED(result))
    {
        *windowSize = { 0, 0, (LONG)winrt::convertDipsToPixels(bounds.Width), (LONG)winrt::convertDipsToPixels(bounds.Height) };
    }

    return result;
}
