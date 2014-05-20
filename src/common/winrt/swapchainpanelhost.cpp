//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// swapchainpanelhost.cpp: Host for managing ISwapChainPanel native window types.

#include "common/winrt/swapchainpanelhost.h"

SwapChainPanelHost::~SwapChainPanelHost()
{
    unregisterForSizeChangeEvents();
}

bool SwapChainPanelHost::initialize(EGLNativeWindowType window)
{
    ComPtr<IInspectable> win = window;
    HRESULT result = win.As(&mSwapChainPanel);
    if (SUCCEEDED(result))
    {
        result = getSwapChainPanelSize(mSwapChainPanel, &mClientRect);
    }

    if (SUCCEEDED(result))
    {
        mNewClientRect = mClientRect;
        mClientRectChanged = false;
        return registerForSizeChangeEvents();
    }

    return false;
}

bool SwapChainPanelHost::registerForSizeChangeEvents()
{
    ComPtr<ABI::Windows::UI::Xaml::ISizeChangedEventHandler> sizeChangedHandler;
    ComPtr<ABI::Windows::UI::Xaml::IFrameworkElement> frameworkElement;
    HRESULT result = Microsoft::WRL::MakeAndInitialize<SwapChainPanelSizeChangedHandler>(sizeChangedHandler.ReleaseAndGetAddressOf(), this->shared_from_this());

    if (SUCCEEDED(result))
    {
        result = mSwapChainPanel.As(&frameworkElement);
    }

    if (SUCCEEDED(result))
    {
        result = frameworkElement->add_SizeChanged(sizeChangedHandler.Get(), &mSizeChangedEventToken);
    }

    if (SUCCEEDED(result))
    {
        return true;
    }

    return false;
}

void SwapChainPanelHost::unregisterForSizeChangeEvents()
{
    ComPtr<ABI::Windows::UI::Xaml::IFrameworkElement> frameworkElement;
    if (SUCCEEDED(mSwapChainPanel.As(&frameworkElement)))
    {
        (void)frameworkElement->remove_SizeChanged(mSizeChangedEventToken);
    }

    mSizeChangedEventToken.value = 0;
}

HRESULT SwapChainPanelHost::createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, unsigned int width, unsigned int height, DXGISwapChain** swapChain)
{
    if (device == NULL || factory == NULL || swapChain == NULL || width == 0 || height == 0)
    {
        return E_INVALIDARG;
    }

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
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

    *swapChain = nullptr;

    ComPtr<IDXGISwapChain1> newSwapChain;
    ComPtr<ISwapChainPanelNative> swapChainPanelNative;

    HRESULT result = factory->CreateSwapChainForComposition(device, &swapChainDesc, nullptr, newSwapChain.ReleaseAndGetAddressOf());

    if (SUCCEEDED(result))
    {
        result = mSwapChainPanel.As(&swapChainPanelNative);
    }

    if (SUCCEEDED(result))
    {
        result = swapChainPanelNative->SetSwapChain(newSwapChain.Get());
    }

    if (SUCCEEDED(result))
    {
        newSwapChain.CopyTo(swapChain);
    }

    return result;
}

HRESULT getSwapChainPanelSize(ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> swapChainPanel, RECT* windowSize)
{
    ComPtr<ABI::Windows::UI::Xaml::IUIElement> uiElement;
    ABI::Windows::Foundation::Size renderSize;
    HRESULT result = swapChainPanel.As(&uiElement);
    if (SUCCEEDED(result))
    {
        result = uiElement->get_RenderSize(&renderSize);
    }

    if (SUCCEEDED(result))
    {
        *windowSize = { 0, 0, (long)renderSize.Width, (long)renderSize.Height };
    }

    return result;
}
