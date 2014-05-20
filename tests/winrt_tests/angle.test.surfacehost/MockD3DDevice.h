//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

class MockD3DDevice : public RuntimeClass<
    RuntimeClassFlags<ClassicCom>,
    ID3D11Device>
{
public:

    MockD3DDevice()
    {
        
    }

    // ID3D11Device
    STDMETHODIMP EnumAdapters(
        UINT Adapter,
        IDXGIAdapter **ppAdapter)
    {
        Assert::Fail(L"Unexpected call to EnumAdapters");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateBuffer(
        const D3D11_BUFFER_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Buffer **ppBuffer)
    {
        Assert::Fail(L"Unexpected call to CreateBuffer");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateTexture1D(
        const D3D11_TEXTURE1D_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Texture1D **ppTexture1D)
    {
        Assert::Fail(L"Unexpected call to CreateTexture1D");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateTexture2D(
        const D3D11_TEXTURE2D_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Texture2D **ppTexture2D)
    {
        Assert::Fail(L"Unexpected call to CreateTexture2D");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateTexture3D(
        const D3D11_TEXTURE3D_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Texture3D **ppTexture3D)
    {
        Assert::Fail(L"Unexpected call to CreateTexture3D");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateShaderResourceView(
        ID3D11Resource *pResource,
        const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,
        ID3D11ShaderResourceView **ppSRView)
    {
        Assert::Fail(L"Unexpected call to CreateShaderResourceView");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateUnorderedAccessView(
        ID3D11Resource *pResource,
        const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc,
        ID3D11UnorderedAccessView **ppUAView)
    {
        Assert::Fail(L"Unexpected call to CreateUnorderedAccessView");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateRenderTargetView(
        ID3D11Resource *pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
        ID3D11RenderTargetView **ppRTView)
    {
        Assert::Fail(L"Unexpected call to CreateRenderTargetView");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateDepthStencilView(
        ID3D11Resource *pResource,
        const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
        ID3D11DepthStencilView **ppDepthStencilView)
    {
        Assert::Fail(L"Unexpected call to CreateDepthStencilView");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateInputLayout(
        const D3D11_INPUT_ELEMENT_DESC *pInputElementDescs,
        UINT NumElements,
        const void *pShaderBytecodeWithInputSignature,
        SIZE_T BytecodeLength,
        ID3D11InputLayout **ppInputLayout)
    {
        Assert::Fail(L"Unexpected call to CreateInputLayout");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateVertexShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11VertexShader **ppVertexShader)
    {
        Assert::Fail(L"Unexpected call to CreateVertexShader");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateGeometryShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11GeometryShader **ppGeometryShader)
    {
        Assert::Fail(L"Unexpected call to CreateGeometryShader");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateGeometryShaderWithStreamOutput(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
        UINT NumEntries,
        const UINT *pBufferStrides,
        UINT NumStrides,
        UINT RasterizedStream,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11GeometryShader **ppGeometryShader)
    {
        Assert::Fail(L"Unexpected call to CreateGeometryShaderWithStreamOutput");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreatePixelShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11PixelShader **ppPixelShader)
    {
        Assert::Fail(L"Unexpected call to CreatePixelShader");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateHullShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11HullShader **ppHullShader)
    {
        Assert::Fail(L"Unexpected call to CreateHullShader");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateDomainShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11DomainShader **ppDomainShader)
    {
        Assert::Fail(L"Unexpected call to CreateDomainShader");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateComputeShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11ComputeShader **ppComputeShader)
    {
        Assert::Fail(L"Unexpected call to CreateComputeShader");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateClassLinkage(
        ID3D11ClassLinkage **ppLinkage)
    {
        Assert::Fail(L"Unexpected call to CreateClassLinkage");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateBlendState(
        const D3D11_BLEND_DESC *pBlendStateDesc,
        ID3D11BlendState **ppBlendState)
    {
        Assert::Fail(L"Unexpected call to CreateBlendState");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateDepthStencilState(
        const D3D11_DEPTH_STENCIL_DESC *pDepthStencilDesc,
        ID3D11DepthStencilState **ppDepthStencilState)
    {
        Assert::Fail(L"Unexpected call to CreateDepthStencilState");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateRasterizerState(
        const D3D11_RASTERIZER_DESC *pRasterizerDesc,
        ID3D11RasterizerState **ppRasterizerState)
    {
        Assert::Fail(L"Unexpected call to CreateRasterizerState");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateSamplerState(
        const D3D11_SAMPLER_DESC *pSamplerDesc,
        ID3D11SamplerState **ppSamplerState)
    {
        Assert::Fail(L"Unexpected call to CreateSamplerState");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateQuery(
        const D3D11_QUERY_DESC *pQueryDesc,
        ID3D11Query **ppQuery)
    {
        Assert::Fail(L"Unexpected call to CreateQuery");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreatePredicate(
        const D3D11_QUERY_DESC *pPredicateDesc,
        ID3D11Predicate **ppPredicate)
    {
        Assert::Fail(L"Unexpected call to CreatePredicate");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateCounter(
        const D3D11_COUNTER_DESC *pCounterDesc,
        ID3D11Counter **ppCounter)
    {
        Assert::Fail(L"Unexpected call to CreateCounter");
        return E_NOTIMPL;
    }

    STDMETHODIMP CreateDeferredContext(
        UINT ContextFlags,
        ID3D11DeviceContext **ppDeferredContext)
    {
        Assert::Fail(L"Unexpected call to CreateDeferredContext");
        return E_NOTIMPL;
    }

    STDMETHODIMP OpenSharedResource(
        HANDLE hResource,
        REFIID ReturnedInterface,
        void **ppResource)
    {
        Assert::Fail(L"Unexpected call to OpenSharedResource");
        return E_NOTIMPL;
    }

    STDMETHODIMP CheckFormatSupport(
        DXGI_FORMAT Format,
        UINT *pFormatSupport)
    {
        Assert::Fail(L"Unexpected call to CheckFormatSupport");
        return E_NOTIMPL;
    }

    STDMETHODIMP CheckMultisampleQualityLevels(
        DXGI_FORMAT Format,
        UINT SampleCount,
        UINT *pNumQualityLevels)
    {
        Assert::Fail(L"Unexpected call to CheckMultisampleQualityLevels");
        return E_NOTIMPL;
    }

    void STDMETHODCALLTYPE CheckCounterInfo(
        D3D11_COUNTER_INFO *pCounterInfo)
    {
        Assert::Fail(L"Unexpected call to CheckCounterInfo");
    }

    STDMETHODIMP CheckCounter(
        const D3D11_COUNTER_DESC *pDesc,
        D3D11_COUNTER_TYPE *pType,
        UINT *pActiveCounters,
        LPSTR szName,
        UINT *pNameLength,
        LPSTR szUnits,
        UINT *pUnitsLength,
        LPSTR szDescription,
        UINT *pDescriptionLength)
    {
        Assert::Fail(L"Unexpected call to CheckCounter");
        return E_NOTIMPL;
    }

    STDMETHODIMP CheckFeatureSupport(
        D3D11_FEATURE Feature,
        void *pFeatureSupportData,
        UINT FeatureSupportDataSize)
    {
        Assert::Fail(L"Unexpected call to CheckFeatureSupport");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetPrivateData(
        REFGUID guid,
        UINT *pDataSize,
        void *pData)
    {
        Assert::Fail(L"Unexpected call to GetPrivateData");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetPrivateData(
        REFGUID guid,
        UINT DataSize,
        const void *pData)
    {
        Assert::Fail(L"Unexpected call to SetPrivateData");
        return E_NOTIMPL;
    }

    STDMETHODIMP SetPrivateDataInterface(
        REFGUID guid,
        const IUnknown *pData)
    {
        Assert::Fail(L"Unexpected call to SetPrivateDataInterface");
        return E_NOTIMPL;
    }

    D3D_FEATURE_LEVEL STDMETHODCALLTYPE GetFeatureLevel(void)
    {
        Assert::Fail(L"Unexpected call to GetFeatureLevel");
        return D3D_FEATURE_LEVEL_11_1;
    }

    UINT STDMETHODCALLTYPE GetCreationFlags(void)
    {
        Assert::Fail(L"Unexpected call to GetCreationFlags");
        return E_NOTIMPL;
    }

    STDMETHODIMP GetDeviceRemovedReason(void)
    {
        Assert::Fail(L"Unexpected call to GetDeviceRemovedReason");
        return E_NOTIMPL;
    }

    void STDMETHODCALLTYPE GetImmediateContext(
        ID3D11DeviceContext **ppImmediateContext)
    {
        Assert::Fail(L"Unexpected call to GetImmediateContext");
    }

    STDMETHODIMP SetExceptionMode(
        UINT RaiseFlags)
    {
        Assert::Fail(L"Unexpected call to SetExceptionMode");
        return E_NOTIMPL;
    }

    UINT STDMETHODCALLTYPE GetExceptionMode(void)
    {
        Assert::Fail(L"Unexpected call to GetExceptionMode");
        return 0;
    }
};