//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

class MockDXGISwapChain : public RuntimeClass<
    RuntimeClassFlags<ClassicCom>,
    IDXGIObject,
    IDXGISwapChain1>
{
public:

    MockDXGISwapChain(const DXGI_SWAP_CHAIN_DESC1 *pDesc)
    {
        mDesc = *pDesc;
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

    // IDXGIDeviceSubObject
    STDMETHODIMP GetDevice(
        REFIID riid,
        void **ppDevice)
    {
        Assert::Fail(L"Unexpected call to GetDevice");
        return E_NOTIMPL;
    }

    // IDXGISwapChain
    STDMETHODIMP Present(
        UINT SyncInterval,
        UINT Flags)
    {
        Assert::Fail(L"Unexpected call to Present");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetBuffer(
        UINT Buffer,
        REFIID riid,
        void **ppSurface)
    {
        Assert::Fail(L"Unexpected call to GetBuffer");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetFullscreenState(
        BOOL Fullscreen,
        IDXGIOutput *pTarget)
    {
        Assert::Fail(L"Unexpected call to SetFullscreenState");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetFullscreenState(
        BOOL *pFullscreen,
        IDXGIOutput **ppTarget)
    {
        Assert::Fail(L"Unexpected call to GetFullscreenState");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetDesc(
        DXGI_SWAP_CHAIN_DESC *pDesc)
    {
        Assert::Fail(L"Unexpected call to GetDesc");
        return E_NOTIMPL;
    }

    STDMETHODIMP ResizeBuffers(
        UINT BufferCount,
        UINT Width,
        UINT Height,
        DXGI_FORMAT NewFormat,
        UINT SwapChainFlags)
    {
        // This Mock when compiled for Windows Phone, should fail the ResizeBuffers( ) call with
        // DXGI_ERROR_UNSUPPORTED to ensure the behavior matches the real device.
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
        return DXGI_ERROR_UNSUPPORTED;
#endif
        Assert::Fail(L"Unexpected call to ResizeBuffers");
        return E_NOTIMPL;
    }

    STDMETHODIMP ResizeTarget(
        const DXGI_MODE_DESC *pNewTargetParameters)
    {
        Assert::Fail(L"Unexpected call to ResizeTarget");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetContainingOutput(
        IDXGIOutput **ppOutput)
    {
        Assert::Fail(L"Unexpected call to GetContainingOutput");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetFrameStatistics(
        DXGI_FRAME_STATISTICS *pStats)
    {
        Assert::Fail(L"Unexpected call to GetFrameStatistics");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetLastPresentCount(
        UINT *pLastPresentCount)
    {
        Assert::Fail(L"Unexpected call to GetLastPresentCount");
        return E_NOTIMPL;
    }

    // IDXGISwapChain1
    STDMETHODIMP GetDesc1(
        DXGI_SWAP_CHAIN_DESC1 *pDesc)
    {
        *pDesc = mDesc;
        return S_OK;
    }

    STDMETHODIMP GetFullscreenDesc(
        DXGI_SWAP_CHAIN_FULLSCREEN_DESC *pDesc)
    {
        Assert::Fail(L"Unexpected call to GetFullscreenDesc");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetHwnd(
        HWND *pHwnd)
    {
        Assert::Fail(L"Unexpected call to GetHwnd");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetCoreWindow(
        REFIID refiid,
        void **ppUnk)
    {
        Assert::Fail(L"Unexpected call to GetCoreWindow");
        return E_NOTIMPL;
    }

    STDMETHODIMP Present1(
        UINT SyncInterval,
        UINT PresentFlags,
        const DXGI_PRESENT_PARAMETERS *pPresentParameters)
    {
        Assert::Fail(L"Unexpected call to Present1");
        return E_NOTIMPL;
    }

    BOOL STDMETHODCALLTYPE IsTemporaryMonoSupported(void)
    {
        Assert::Fail(L"Unexpected call to IsTemporaryMonoSupported");
        return FALSE;
    }

    STDMETHODIMP GetRestrictToOutput(
        IDXGIOutput **ppRestrictToOutput)
    {
        Assert::Fail(L"Unexpected call to GetRestrictToOutput");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetBackgroundColor(
        const DXGI_RGBA *pColor)
    {
        Assert::Fail(L"Unexpected call to SetBackgroundColor");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetBackgroundColor(
        DXGI_RGBA *pColor)
    {
        Assert::Fail(L"Unexpected call to GetBackgroundColor");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetRotation(
        DXGI_MODE_ROTATION Rotation)
    {
        Assert::Fail(L"Unexpected call to SetRotation");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetRotation(
        DXGI_MODE_ROTATION *pRotation)
    {
        Assert::Fail(L"Unexpected call to GetRotation");
        return E_NOTIMPL;
    }

private:

    DXGI_SWAP_CHAIN_DESC1 mDesc;

};