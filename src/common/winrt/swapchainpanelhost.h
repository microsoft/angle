//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// swapchainpanelhost.h: Host for managing ISwapChainPanel native window types.

#ifndef COMMON_SWAPCHAINPANELHOST_H_
#define COMMON_SWAPCHAINPANELHOST_H_

#include "common/winrt/iinspectablehost.h"

class SwapChainPanelHost : public IInspectableSurfaceHost, public std::enable_shared_from_this<SwapChainPanelHost>
{
public:
    ~SwapChainPanelHost();
    bool initialize(EGLNativeWindowType window, IPropertySet* propertySet);
    bool registerForSizeChangeEvents();
    void unregisterForSizeChangeEvents();
    HRESULT createSwapChain(ID3D11Device* device, DXGIFactory* factory, DXGI_FORMAT format, unsigned int width, unsigned int height, DXGISwapChain** swapChain);
    HRESULT scaleSwapChain(SIZE& newSize);
private:
    ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel> mSwapChainPanel;
    ComPtr<IMap<HSTRING, IInspectable*>> mPropertyMap;
    ComPtr<DXGISwapChain> mSwapChain;
};

[uuid(8ACBD974-8187-4508-AD80-AEC77F93CF36)]
class SwapChainPanelSizeChangedHandler :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, ABI::Windows::UI::Xaml::ISizeChangedEventHandler>
{
public:
    SwapChainPanelSizeChangedHandler() { }
    HRESULT RuntimeClassInitialize(std::shared_ptr<IInspectableSurfaceHost> host)
    {
        if (!host)
        {
            return E_INVALIDARG;
        }

        mHost = host;
        return S_OK;
    }

    // ISizeChangedEventHandler
    IFACEMETHOD(Invoke)(IInspectable *sender, ABI::Windows::UI::Xaml::ISizeChangedEventArgs *e)
    {
        std::shared_ptr<IInspectableSurfaceHost> host = mHost.lock();
        if (host)
        {
            // The size of the ISwapChainPanel control is returned in DIPs.
            // We are keeping these in dips because the swapchain created for composition
            // also uses dip units. This keeps dimensions, viewports, etc in the same unit.
            // XAML Clients of the ISwapChainPanel are required to use dips to define their
            // layout sizes as well.
            ABI::Windows::Foundation::Size newSize;
            HRESULT result = e->get_NewSize(&newSize);
            if (SUCCEEDED(result))
            {
                SIZE windowSize = { lround(newSize.Width), lround(newSize.Height) };
                host->setNewClientSize(windowSize);
            }
        }

        return S_OK;
    }

private:
    std::weak_ptr<IInspectableSurfaceHost> mHost;
};

HRESULT getSwapChainPanelSize(const ComPtr<ABI::Windows::UI::Xaml::Controls::ISwapChainPanel>& swapChainPanel, RECT* windowSize);

#endif // COMMON_SWAPCHAINPANELHOST_H_
