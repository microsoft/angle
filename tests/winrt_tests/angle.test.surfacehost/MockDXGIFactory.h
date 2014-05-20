//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

class MockDXGIFactory : public RuntimeClass<
    RuntimeClassFlags<ClassicCom>,
    IDXGIObject,
    IDXGIFactory,
    IDXGIFactory1,
    IDXGIFactory2>
{
public:

    MockDXGIFactory()
    {
        ZeroMemory(&mDesc, sizeof(mDesc));
    }

    // MockDXGIFactory
    void GetDescUsed(DXGI_SWAP_CHAIN_DESC1& desc)
    {
        desc = mDesc;
    }

    // IDXGIObject
    STDMETHODIMP SetPrivateData(
        REFGUID Name,
        UINT DataSize,
        const void *pData)
    {
        Assert::Fail(L"Unexpected call to SetPrivateData");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetPrivateDataInterface(
        REFGUID Name,
        const IUnknown *pUnknown)
    {
        Assert::Fail(L"Unexpected call to SetPrivateDataInterface");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetPrivateData(
        REFGUID Name,
        UINT *pDataSize,
        void *pData)
    {
        Assert::Fail(L"Unexpected call to GetPrivateData");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetParent(
        REFIID riid,
        void **ppParent)
    {
        Assert::Fail(L"Unexpected call to GetParent");
        return E_NOTIMPL;
    }

    // IDXGIFactory
    STDMETHODIMP EnumAdapters(
        UINT Adapter,
        IDXGIAdapter **ppAdapter)
    {
        Assert::Fail(L"Unexpected call to EnumAdapters");
        return E_NOTIMPL;
    }

    STDMETHODIMP MakeWindowAssociation(
        HWND WindowHandle,
        UINT Flags)
    {
        Assert::Fail(L"Unexpected call to MakeWindowAssociation");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetWindowAssociation(
        HWND *pWindowHandle)
    {
        Assert::Fail(L"Unexpected call to GetWindowAssociation");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateSwapChain(
        IUnknown *pDevice,
        DXGI_SWAP_CHAIN_DESC *pDesc,
        IDXGISwapChain **ppSwapChain)
    {
        Assert::Fail(L"Unexpected call to CreateSwapChain");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateSoftwareAdapter(
        HMODULE Module,
        IDXGIAdapter **ppAdapter)
    {
        Assert::Fail(L"Unexpected call to CreateSoftwareAdapter");
        return E_NOTIMPL;
    }

    // IDXGIFactory1
    STDMETHODIMP EnumAdapters1(
        UINT Adapter,
        IDXGIAdapter1 **ppAdapter)
    {
        Assert::Fail(L"Unexpected call to EnumAdapters1");
        return E_NOTIMPL;
    }

    BOOL STDMETHODCALLTYPE IsCurrent(void)
    {
        Assert::Fail(L"Unexpected call to IsCurrent");
        return FALSE;
    }

    // IDXGIFactory2
    BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled(void)
    {
        Assert::Fail(L"Unexpected call to IsWindowedStereoEnabled");
        return FALSE;
    }

    STDMETHODIMP CreateSwapChainForHwnd(
        IUnknown *pDevice,
        HWND hWnd,
        const DXGI_SWAP_CHAIN_DESC1 *pDesc,
        const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pFullscreenDesc,
        IDXGIOutput *pRestrictToOutput,
        IDXGISwapChain1 **ppSwapChain)
    {
        Assert::Fail(L"Unexpected call to EnumAdapters1");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateSwapChainForCoreWindow(
        IUnknown *pDevice,
        IUnknown *pWindow,
        const DXGI_SWAP_CHAIN_DESC1 *pDesc,
        IDXGIOutput *pRestrictToOutput,
        IDXGISwapChain1 **ppSwapChain)
    {
        ComPtr<IDXGISwapChain1> swapChain = Make<MockDXGISwapChain>(pDesc);
        Assert::IsNotNull(swapChain.Get(), L"Unexpected memory allocation failure for MockDXGISwapChain");
        *ppSwapChain = swapChain.Detach();
        return S_OK;
    }

    STDMETHODIMP GetSharedResourceAdapterLuid(
        HANDLE hResource,
        LUID *pLuid)
    {
        Assert::Fail(L"Unexpected call to GetSharedResourceAdapterLuid");
        return E_NOTIMPL;
    }

    STDMETHODIMP RegisterStereoStatusWindow(
        HWND WindowHandle,
        UINT wMsg,
        DWORD *pdwCookie)
    {
        Assert::Fail(L"Unexpected call to RegisterStereoStatusWindow");
        return E_NOTIMPL;
    }

    STDMETHODIMP RegisterStereoStatusEvent(
        HANDLE hEvent,
        DWORD *pdwCookie)
    {
        Assert::Fail(L"Unexpected call to RegisterStereoStatusEvent");
        return E_NOTIMPL;
    }

    void STDMETHODCALLTYPE UnregisterStereoStatus(
        DWORD dwCookie)
    {
        Assert::Fail(L"Unexpected call to UnregisterStereoStatus");
    }

    STDMETHODIMP RegisterOcclusionStatusWindow(
        HWND WindowHandle,
        UINT wMsg,
        DWORD *pdwCookie)
    {
        Assert::Fail(L"Unexpected call to RegisterOcclusionStatusWindow");
        return E_NOTIMPL;
    }

    STDMETHODIMP RegisterOcclusionStatusEvent(
        HANDLE hEvent,
        DWORD *pdwCookie)
    {
        Assert::Fail(L"Unexpected call to RegisterOcclusionStatusEvent");
        return E_NOTIMPL;
    }

    void STDMETHODCALLTYPE UnregisterOcclusionStatus(
        DWORD dwCookie)
    {
        Assert::Fail(L"Unexpected call to UnregisterOcclusionStatus");
    }

    STDMETHODIMP CreateSwapChainForComposition(
        IUnknown *pDevice,
        const DXGI_SWAP_CHAIN_DESC1 *pDesc,
        IDXGIOutput *pRestrictToOutput,
        IDXGISwapChain1 **ppSwapChain)
    {
        ComPtr<IDXGISwapChain1> swapChain = Make<MockDXGISwapChain>(pDesc);
        Assert::IsNotNull(swapChain.Get(), L"Unexpected memory allocation failure for MockDXGISwapChain");
        *ppSwapChain = swapChain.Detach();
        return S_OK;
    }

private:

    DXGI_SWAP_CHAIN_DESC1 mDesc;

};