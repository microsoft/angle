//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HolographicSwapChain11.cpp: Implements a back-end specific class for a swap chain that is managed by Windows Holographic.

#include "libANGLE/Context.h"
#include "libANGLE/validationES.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/renderer/d3d/d3d11/winrt/HolographicSwapChain11.h"

#include <windows.graphics.directx.direct3d11.interop.h>

#include <directxmath.h>
#include <windowsnumerics.h>

#include <EGL/eglext.h>

#include "libANGLE/features.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/NativeWindow.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "third_party/trace_event/trace_event.h"

// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthrough2d11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/passthroughrgba2d11ps.h"

#include "libANGLE/renderer/d3d/d3d11/winrt/DepthBufferPlaneFinder/DepthBufferPlaneFinder.h"


#ifdef ANGLE_ENABLE_KEYEDMUTEX
#define ANGLE_RESOURCE_SHARE_TYPE D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX
#else
#define ANGLE_RESOURCE_SHARE_TYPE D3D11_RESOURCE_MISC_SHARED
#endif

using namespace Microsoft::WRL;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace ABI::Windows::Graphics::DirectX::Direct3D11;
using namespace ABI::Windows::Graphics::Holographic;
using namespace ABI::Windows::Perception::Spatial;
using namespace DirectX;

namespace rx
{

namespace
{
bool NeedsOffscreenTexture(Renderer11 *renderer, NativeWindow nativeWindow, EGLint orientation)
{
    // We don't need an offscreen texture if either orientation = INVERT_Y,
    // or present path fast is enabled and we're not rendering onto an offscreen surface.
    return orientation != EGL_SURFACE_ORIENTATION_INVERT_Y_ANGLE &&
           !(renderer->presentPathFastEnabled() && nativeWindow.getNativeWindow());
}
}  // anonymous namespace


DirectX::XMFLOAT4X4 HolographicSwapChain11::mMidViewMatrix;
DirectX::XMFLOAT4X4 HolographicSwapChain11::mMidViewMatrixInverse;
bool HolographicSwapChain11::mUseAutomaticStereoRendering              = true;
bool HolographicSwapChain11::mUseAutomaticDepthBasedImageStabilization = false;
bool HolographicSwapChain11::mWaitForVBlank = true;


HolographicSwapChain11::HolographicSwapChain11(Renderer11 *renderer,
                         HolographicNativeWindow* nativeWindow,
                         HANDLE shareHandle,
                         ABI::Windows::Graphics::Holographic::IHolographicCamera* pCamera)
    : SwapChainD3D(*((NativeWindow*)nativeWindow), shareHandle, GL_RGBA, GL_DEPTH_COMPONENT16),
      mHolographicNativeWindow(nativeWindow),
      mRenderer(renderer),
      mHolographicCamera(pCamera),
      mBackBufferTexture(nullptr),
      mBackBufferRTView(nullptr),
      mBackBufferSRView(nullptr),
      mDepthStencilTexture(nullptr),
      mDepthStencilDSView(nullptr),
      mDepthStencilSRView(nullptr),
      mDepthStencilSRViewMono(nullptr),
      mResolvedDepthBuffer(nullptr),
      mCPUResolvedDepthTexture(nullptr),
      mResolvedDepthView(nullptr),
      mResolvedDepthBufferMappable(nullptr),
      mResolvedDepthViewMappable(nullptr),
      mColorRenderTarget(this, renderer, false),
      mDepthStencilRenderTarget(this, renderer, true)
{
    mHolographicCamera->get_RenderTargetSize(&mRenderTargetSize);
    mHolographicCamera->get_ViewportScaleFactor(&mViewportScaleFactor);
    mHolographicCamera->get_IsStereo(&mIsStereo);

    // cache the ID
    mHolographicCamera->get_Id(&mHolographicCameraId);
    mHolographicCamera->SetNearPlaneDistance(mNearPlaneDistance);
    mHolographicCamera->SetFarPlaneDistance(mFarPlaneDistance);

    XMStoreFloat4x4(&mMidViewMatrix, XMMatrixIdentity());
    XMStoreFloat4x4(&mMidViewMatrixInverse, XMMatrixIdentity());

    InitDepthCurveArray(
        mNearPlaneDistance,
        mFarPlaneDistance,
        mNearPlaneDistance > mFarPlaneDistance ? 0 : UINT16_MAX,
        mDepthWeightArray);

    mDepthBufferPlaneFinder = std::make_unique<HolographicDepthBasedImageStabilization::DepthBufferPlaneFinder>(mRenderer);
}

HolographicSwapChain11::~HolographicSwapChain11()
{
    release();
    mDepthBufferPlaneFinder->ReleaseDeviceDependentResources();
}

void HolographicSwapChain11::release()
{
    SafeRelease(mKeyedMutex);

    // Release references to the back buffer
    mBackBufferRTView.Reset();
    mBackBufferSRView.Reset();
    
    // Force the back buffer to be released entirely
    mBackBufferTexture.Reset();

    releaseOffscreenDepthBuffer();
}

void HolographicSwapChain11::releaseOffscreenDepthBuffer()
{
    SafeRelease(mDepthStencilTexture);
    SafeRelease(mDepthStencilDSView);
    SafeRelease(mDepthStencilSRView);
    SafeRelease(mDepthStencilSRViewMono);
    SafeRelease(mResolvedDepthBuffer);
    SafeRelease(mCPUResolvedDepthTexture);
    SafeRelease(mResolvedDepthView);
    SafeRelease(mResolvedDepthBufferMappable);
    SafeRelease(mResolvedDepthViewMappable);
}

EGLint HolographicSwapChain11::resetOffscreenBuffers(int backbufferWidth, int backbufferHeight)
{
    EGLint result = resetOffscreenDepthBuffer(backbufferWidth, backbufferHeight);
    if (result != EGL_SUCCESS)
    {
        return result;
    }

    return EGL_SUCCESS;
}

EGLint HolographicSwapChain11::resetOffscreenDepthBuffer(int backbufferWidth, int backbufferHeight)
{
    releaseOffscreenDepthBuffer();

    HRESULT result = S_OK;

    if (mDepthBufferFormat != GL_NONE)
    {
        const d3d11::TextureFormat &depthBufferFormatInfo =
            d3d11::GetTextureFormatInfo(mDepthBufferFormat, mRenderer->getRenderer11DeviceCaps());

        CD3D11_TEXTURE2D_DESC depthStencilTextureDesc(
            depthBufferFormatInfo.texFormat,
            backbufferWidth,
            backbufferHeight,
            mIsStereo ? 2 : 1,
            1,
            D3D11_BIND_DEPTH_STENCIL);

        if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
        {
            depthStencilTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }

        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;

        ID3D11Device *device = mRenderer->getDevice();
        result = device->CreateTexture2D(&depthStencilTextureDesc, NULL, &mDepthStencilTexture);
        if (FAILED(result))
        {
            ERR("Could not create depthstencil surface for new swap chain: 0x%08X", result);
            release();

            if (d3d11::isDeviceLostError(result))
            {
                return EGL_CONTEXT_LOST;
            }
            else
            {
                return EGL_BAD_ALLOC;
            }
        }
        d3d11::SetDebugName(mDepthStencilTexture, "Offscreen depth stencil texture");

        CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc(
            mIsStereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D,
            depthBufferFormatInfo.dsvFormat,
            0,
            0,
            mIsStereo ? 2 : 1, // Not used if the DSV dimension is TEXTURE2D
            0);

        result = device->CreateDepthStencilView(mDepthStencilTexture, &depthStencilDesc, &mDepthStencilDSView);
        ASSERT(SUCCEEDED(result));
        d3d11::SetDebugName(mDepthStencilDSView, "Offscreen depth stencil view");

        if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
        {
            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc;
                depthStencilSRVDesc.Format = depthBufferFormatInfo.srvFormat;
                depthStencilSRVDesc.ViewDimension = mIsStereo ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
                depthStencilSRVDesc.Texture2D.MostDetailedMip = 0;
                depthStencilSRVDesc.Texture2D.MipLevels = static_cast<UINT>(-1);
                depthStencilSRVDesc.Texture2DArray.ArraySize = mIsStereo ? 2 : 1; // Not used if the DSV dimension is TEXTURE2D
                depthStencilSRVDesc.Texture2DArray.FirstArraySlice = 0;
                depthStencilSRVDesc.Texture2DArray.MostDetailedMip = 0;
                depthStencilSRVDesc.Texture2DArray.MipLevels = static_cast<UINT>(-1);

                result = device->CreateShaderResourceView(mDepthStencilTexture, &depthStencilSRVDesc, &mDepthStencilSRView);
                ASSERT(SUCCEEDED(result));
                d3d11::SetDebugName(mDepthStencilSRView, "Offscreen depth stencil shader resource");
            }

            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDescMonoLeft(
                    D3D11_SRV_DIMENSION_TEXTURE2D,
                    depthBufferFormatInfo.srvFormat,
                    0,
                    static_cast<UINT>(-1));

                result = device->CreateShaderResourceView(mDepthStencilTexture, &depthStencilSRVDescMonoLeft, &mDepthStencilSRViewMono);
                ASSERT(SUCCEEDED(result));
                d3d11::SetDebugName(mDepthStencilSRViewMono, "Offscreen depth stencil shader resource (mono)");
            }
        }

        // Determine whether current Direct3D device can support Mappable Default Buffers functionality. If not, fall back to
        // using Staging Buffers to access data on the CPU.
        D3D11_FEATURE_DATA_D3D11_OPTIONS1 featureOptions;
        result = device->CheckFeatureSupport(
            D3D11_FEATURE_D3D11_OPTIONS1,
            &featureOptions,
            sizeof(featureOptions));

        bool depthBufferMappedOnGpu = featureOptions.MapOnDefaultBuffers ? true : false;

        const unsigned int outWidth  = lround(backbufferWidth) / DEPTH_BUFFER_SUBSAMPLE_RATE;
        const unsigned int outHeight = lround(backbufferHeight) / DEPTH_BUFFER_SUBSAMPLE_RATE;
        const unsigned int structureByteStride = sizeof(uint16_t);
        const unsigned int numPixels = outWidth * outHeight;
        const unsigned int byteWidth = structureByteStride * numPixels;
        if (depthBufferMappedOnGpu)
        {
            // Create a default buffer resource that can be mapped on CPU.
            CD3D11_BUFFER_DESC depthResolvedDesc(
                byteWidth,
                D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
                D3D11_USAGE_DEFAULT,
                D3D11_CPU_ACCESS_READ,
                0,
                structureByteStride);

            if (SUCCEEDED(result))
            {
                result = device->CreateBuffer(&depthResolvedDesc, nullptr, &mResolvedDepthBufferMappable);
            }

            CD3D11_UNORDERED_ACCESS_VIEW_DESC depthResolvedUAVDesc(
                D3D11_UAV_DIMENSION_BUFFER,
                DXGI_FORMAT_R16_UNORM,
                0,
                numPixels);
            
            if (SUCCEEDED(result))
            {
                result = device->CreateUnorderedAccessView(
                    mResolvedDepthBufferMappable,
                    &depthResolvedUAVDesc,
                    &mResolvedDepthViewMappable);
            }
        }
        else
        {
            // Create a default resource, and copy it to a staging buffer for CPU read.
            CD3D11_BUFFER_DESC depthResolvedDesc(
                byteWidth,
                D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS,
                D3D11_USAGE_DEFAULT,
                0,
                0,
                structureByteStride);
            
            if (SUCCEEDED(result))
            {
                result = device->CreateBuffer(&depthResolvedDesc, nullptr, &mResolvedDepthBuffer);
            }
            
            CD3D11_UNORDERED_ACCESS_VIEW_DESC depthResolvedUAVDesc(
                D3D11_UAV_DIMENSION_BUFFER,
                DXGI_FORMAT_R16_UNORM,
                0,
                numPixels);
            
            if (SUCCEEDED(result))
            {
                result = device->CreateUnorderedAccessView(
                    mResolvedDepthBuffer, 
                    &depthResolvedUAVDesc, 
                    &mResolvedDepthView);
            }

            CD3D11_BUFFER_DESC depthResolvedCPUDesc(
                byteWidth,
                0,
                D3D11_USAGE_STAGING,
                D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE,
                0,
                structureByteStride);
            
            if (SUCCEEDED(result))
            {
                result = device->CreateBuffer(
                    &depthResolvedCPUDesc, 
                    nullptr, 
                    &mCPUResolvedDepthTexture);
            }
        }
    }

    if (SUCCEEDED(result))
    {
        return EGL_SUCCESS;
    }
    else
    {
        return EGL_BAD_ALLOC;
    }
}

EGLint HolographicSwapChain11::resize(EGLint backbufferWidth, EGLint backbufferHeight)
{
    TRACE_EVENT0("gpu.angle", "SwapChain11::resize");
    ID3D11Device *device = mRenderer->getDevice();

    if (device == NULL)
    {
        return EGL_BAD_ACCESS;
    }

    // Holographic apps do not have access to resize the display.
    return EGL_SUCCESS;
}


void HolographicSwapChain11::ComputeMidViewMatrix(
    const XMMATRIX& left,
    const XMMATRIX& right)
{
    // Decompose both matrices.
    XMVECTOR leftScale, leftRotationQuat, leftTranslation;
    bool success = XMMatrixDecompose(&leftScale, &leftRotationQuat, &leftTranslation, left);

    XMVECTOR rightScale, rightRotationQuat, rightTranslation;
    success = success && XMMatrixDecompose(&rightScale, &rightRotationQuat, &rightTranslation, right);
    
    // Only proceed if the decomposition was successful.
    if (success)
    {
        // LERP the scale.
        const XMVECTOR resultScale = XMVectorLerp(leftScale, rightScale, 0.5f);

        // LERP the position.
        const XMVECTOR resultTranslation = XMVectorLerp(leftTranslation, rightTranslation, 0.5f);

        // SLERP the rotation.
        const XMVECTOR resultRotationQuat = XMQuaternionSlerp(leftRotationQuat, rightRotationQuat, 0.5f);

        // Compose the result transform.
        auto midViewMatrix = 
            XMMatrixScalingFromVector(resultScale) * 
            XMMatrixRotationQuaternion(resultRotationQuat) * 
            XMMatrixTranslationFromVector(resultTranslation);

        // Store the result transform.
        DirectX::XMStoreFloat4x4(&mMidViewMatrix, midViewMatrix);
    }
}

EGLint HolographicSwapChain11::updateHolographicRenderingParameters(
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters>& spCameraRenderingParameters)
{
    TRACE_EVENT0("gpu.angle", "HolographicSwapChain11::updateHolographicRenderingParameters");

    if (mRenderer == NULL)
    {
        return EGL_BAD_ACCESS;
    }

    ID3D11Device *device = mRenderer->getDevice();

    if (device == NULL)
    {
        return EGL_BAD_ACCESS;
    }

    HRESULT result = S_OK;

    // Update back buffer resources.
    {
        // Get the WinRT object representing the holographic camera's back buffer.
        ComPtr<IDirect3DSurface> spSurface;
        if (SUCCEEDED(result))
        {
            result = spCameraRenderingParameters->get_Direct3D11BackBuffer(spSurface.GetAddressOf());
        }

        // Get a DXGI interface for the holographic camera's back buffer.
        // Holographic cameras do not provide the DXGI swap chain, which is owned
        // by the system. The Direct3D back buffer resource is provided using WinRT
        // interop APIs.
        ComPtr<IInspectable> spInspectable;
        if (SUCCEEDED(result))
        {
            result = spSurface.As(&spInspectable);
        }
        ComPtr<IDirect3DDxgiInterfaceAccess> spDxgiInterfaceAccess;
        if (SUCCEEDED(result))
        {
            result = spInspectable.As(&spDxgiInterfaceAccess);
        }
        ComPtr<ID3D11Resource> spResource;
        if (SUCCEEDED(result))
        {
            result = spDxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&spResource));
        }
        // Get a Direct3D interface for the holographic camera's back buffer.
        ComPtr<ID3D11Texture2D> spCameraBackBuffer;
        if (SUCCEEDED(result))
        {
            result = spResource.As(&spCameraBackBuffer);
        }

        // Don't resize unnecessarily
        if (mBackBufferTexture != spCameraBackBuffer)
        {
            // Can only recreate views if we have a resource
            ASSERT(spCameraBackBuffer);

            mBackBufferTexture.Reset();
            mBackBufferRTView.Reset();
            mBackBufferSRView.Reset();

            // This can change every frame as the system moves to the next buffer in the
            // swap chain. This mode of operation will occur when certain rendering modes
            // are activated.
            mBackBufferTexture = spCameraBackBuffer;

            // Create a render target view of the back buffer.
            // Creating this resource is inexpensive, and is better than keeping track of
            // the back buffers in order to pre-allocate render target views for each one.
            d3d11::SetDebugName(mBackBufferTexture.Get(), "Back buffer texture");
            result = device->CreateRenderTargetView(mBackBufferTexture.Get(), NULL, &mBackBufferRTView);
            ASSERT(SUCCEEDED(result));
            if (SUCCEEDED(result))
            {
                d3d11::SetDebugName(mBackBufferRTView.Get(), "Back buffer render target");
            }

            // Get the DXGI format for the back buffer.
            // This information can be accessed by the app using CameraResources::GetBackBufferDXGIFormat().
            D3D11_TEXTURE2D_DESC backBufferDesc;
            mBackBufferTexture->GetDesc(&backBufferDesc);
            mDxgiFormat = backBufferDesc.Format;

            // Check for render target size changes.
            ABI::Windows::Foundation::Size currentSize;
            result = mHolographicCamera->get_RenderTargetSize(&currentSize);
            if (SUCCEEDED(result))
            {
                if (mRenderTargetSize.Height != currentSize.Height ||
                    mRenderTargetSize.Width != currentSize.Width)
                {
                    // Set render target size.
                    mRenderTargetSize = currentSize;
                }
            }

            if (SUCCEEDED(result))
            {
                result = device->CreateRenderTargetView(
                    mBackBufferTexture.Get(),
                    nullptr,
                    &mBackBufferRTView
                );
            }

            if (SUCCEEDED(result) && (backBufferDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE))
            {
                CD3D11_SHADER_RESOURCE_VIEW_DESC desc(
                    mIsStereo ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D,
                    backBufferDesc.Format);
                HRESULT hr = device->CreateShaderResourceView(mBackBufferTexture.Get(), &desc, &mBackBufferSRView);

                if (FAILED(hr))
                {
                    return EGL_BAD_ALLOC;
                }

                if (mBackBufferSRView != nullptr)
                {
                    d3d11::SetDebugName(mBackBufferSRView.Get(), "Back buffer shader resource");
                }
            }

            // A new depth stencil view is also needed.
            return resetOffscreenBuffers(
                static_cast<EGLint>(mRenderTargetSize.Width),
                static_cast<EGLint>(mRenderTargetSize.Height));
        }
    }

    // Update holographic view/projection matrices.
    if (SUCCEEDED(result))
    {
        ComPtr<ABI::Windows::Perception::Spatial::ISpatialCoordinateSystem> spCoordinateSystem = 
            mHolographicNativeWindow->GetCoordinateSystem();

        if (spCoordinateSystem != nullptr)
        {
            ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraPose> spPose;
            result = mHolographicNativeWindow->GetHolographicCameraPoses()->GetAt(mHolographicCameraId, spPose.GetAddressOf());

            ABI::Windows::Graphics::Holographic::HolographicStereoTransform viewTransform;
            if (SUCCEEDED(result))
            {
                ComPtr<IReference<ABI::Windows::Graphics::Holographic::HolographicStereoTransform>> spViewTransformReference;
                result = spPose->TryGetViewTransform(spCoordinateSystem.Get(), spViewTransformReference.GetAddressOf());

                if (SUCCEEDED(result) && (spViewTransformReference != nullptr))
                {
                    result = spViewTransformReference->get_Value(&viewTransform);
                }
            }

            ABI::Windows::Graphics::Holographic::HolographicStereoTransform projectionTransform;
            if (SUCCEEDED(result))
            {
                result = spPose->get_ProjectionTransform(&projectionTransform);
            }

            if (SUCCEEDED(result))
            {
                // Update the view matrices. Holographic cameras (such as Microsoft HoloLens) are
                // constantly moving relative to the world. The view matrices need to be updated
                // every frame.
                DirectX::XMFLOAT4X4 viewProj[2];
                DirectX::XMStoreFloat4x4(
                    &viewProj[0],
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Left)) * 
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Left))
                );
                DirectX::XMStoreFloat4x4(
                    &viewProj[1],
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Right)) *
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Right))
                );

                // get view matrix
                const auto leftViewMatrix = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Left));
                const auto rightViewMatrix = DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Right));
                
                // interpolate view matrix
                if (mHolographicCameraId == 0)
                {
                    ComputeMidViewMatrix(leftViewMatrix, rightViewMatrix);
                }
                
                // TODO: The display was being mirrored, so for now we hack these values to 
                //       negative as far as the app is concerned.
                viewTransform.Left.M11 = -viewTransform.Left.M11;
                viewTransform.Left.M12 = -viewTransform.Left.M12;
                viewTransform.Left.M13 = -viewTransform.Left.M13;
                viewTransform.Right.M11 = -viewTransform.Right.M11;
                viewTransform.Right.M12 = -viewTransform.Right.M12;
                viewTransform.Right.M13 = -viewTransform.Right.M13;
                mMidViewMatrix._11 = -mMidViewMatrix._11;
                mMidViewMatrix._12 = -mMidViewMatrix._12;
                mMidViewMatrix._13 = -mMidViewMatrix._13;

                // store view matrix
                DirectX::XMFLOAT4X4 view[2];
                DirectX::XMStoreFloat4x4(
                    &view[0],
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Left))
                );
                DirectX::XMStoreFloat4x4(
                    &view[1],
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Right))
                );
                
                // invert view matrix
                XMVECTOR determinant;
                auto leftViewInverse = XMMatrixInverse(&determinant, leftViewMatrix);

                // store view matrix - and its inverse - for the depth-based focus plane finder
                DirectX::XMStoreFloat4x4(&mView, leftViewMatrix);
                if (!XMVector4NearEqual(determinant, XMVectorZero(), XMVectorSplatEpsilon()))
                {
                    DirectX::XMStoreFloat4x4(&mViewInverse, leftViewInverse);
                }

                // get projection
                DirectX::XMFLOAT4X4 proj[2];
                DirectX::XMStoreFloat4x4(
                    &proj[0],
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Left))
                );
                DirectX::XMStoreFloat4x4(
                    &proj[1],
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Right))
                );
                DirectX::XMStoreFloat4x4(
                    &mProjection,
                    DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Left))
                );

                // get the current program
                gl::Context *glContext = gl::GetValidGlobalContext();
                gl::Program *program = glContext->getState().getProgram();

                if (program)
                {
                    // attach holographic view matrix, if applicable
                    int viewMatrixIndex = program->getUniformLocation("uHolographicViewMatrix");
                    if (viewMatrixIndex != -1)
                    {
                        // detach any existing buffers
                        glContext->bindGenericUniformBuffer(0);

                        // attach holographic view matrix
                        if (!(gl::ValidateUniformMatrix(glContext, GL_FLOAT_MAT4, viewMatrixIndex, 2, GL_FALSE)))
                        {
                            // Something unexpected has occured. We're probably in a bad state and should not continue.
                            result = E_FAIL;
                        }
                        else
                        {
                            program->setUniformMatrix4fv(viewMatrixIndex, 2, GL_FALSE, (GLfloat*) view);
                        }
                    }

                    // attach holographic projection matrix, if applicable
                    int projectionMatrixIndex = program->getUniformLocation("uHolographicProjectionMatrix");
                    if (projectionMatrixIndex != -1)
                    {
                        // detach any existing buffers
                        glContext->bindGenericUniformBuffer(0);

                        // attach holographic projection matrix
                        if (!(gl::ValidateUniformMatrix(glContext, GL_FLOAT_MAT4, projectionMatrixIndex, 2, GL_FALSE)))
                        {
                            // Something unexpected has occured. We're probably in a bad state and should not continue.
                            result = E_FAIL;
                        }
                        else
                        {
                            program->setUniformMatrix4fv(projectionMatrixIndex, 2, GL_FALSE, (GLfloat*) proj);
                        }
                    }

                    // attach holographic view/projection matrix, if applicable
                    GLint viewProjectionMatrixUniformLocation = program->getUniformLocation("uHolographicViewProjectionMatrix");
                    if (viewProjectionMatrixUniformLocation != -1)
                    {
                        // detach any existing buffers
                        glContext->bindGenericUniformBuffer(0);

                        // attach holographic view/projection matrix
                        if (!(gl::ValidateUniformMatrix(glContext, GL_FLOAT_MAT4, viewProjectionMatrixUniformLocation, 2, GL_FALSE)))
                        {
                            // Something unexpected has occured. We're probably in a bad state and should not continue.
                            result = E_FAIL;
                        }
                        program->setUniformMatrix4fv(viewProjectionMatrixUniformLocation, 2, GL_FALSE, (GLfloat*) viewProj);
                    }

                    // attach the mid-view matrix inverse, when enabled
                    if (mUseAutomaticStereoRendering)
                    {
                        DirectX::XMVECTOR determinant;
                        auto midViewInverse = XMMatrixInverse(&determinant, XMLoadFloat4x4(&mMidViewMatrix));

                        if (!XMVector4NearEqual(determinant, XMVectorZero(), XMVectorSplatEpsilon()))
                        {
                            XMStoreFloat4x4(&mMidViewMatrixInverse, midViewInverse);
                        }

                        GLint undoViewProjUniformLocation = program->getUniformLocation("uUndoMidViewMatrix");
                        if (undoViewProjUniformLocation != -1)
                        {
                            // detach any existing buffers
                            glContext->bindGenericUniformBuffer(0);

                            if (!(gl::ValidateUniformMatrix(glContext, GL_FLOAT_MAT4, undoViewProjUniformLocation, 1, GL_FALSE)))
                            {
                                // Something unexpected has occured. We're probably in a bad state and should not continue.
                                result = E_FAIL;
                            }
                            program->setUniformMatrix4fv(undoViewProjUniformLocation, 1, GL_FALSE, (GLfloat*) &mMidViewMatrixInverse);
                        }
                    }
                }
            }
        }
    }

    if (FAILED(result))
    {
        ERR("Error reacquiring swap chain buffers: 0x%08X", result);
        release();

        if (d3d11::isDeviceLostError(result))
        {
            return EGL_CONTEXT_LOST;
        }
        else
        {
            return EGL_BAD_ALLOC;
        }
    }
    else
    {
        return EGL_SUCCESS;
    }
}

DXGI_FORMAT HolographicSwapChain11::getSwapChainNativeFormat() const
{
    // Return the format of the swap chain provided by Windows Holographic.
    return mDxgiFormat;
}

EGLint HolographicSwapChain11::reset(int backbufferWidth, int backbufferHeight, EGLint swapInterval)
{
    // Windows Holographic apps use APIs to access the back buffer.
    UINT32 id = mHolographicCameraId;

    {
        HRESULT hr = S_OK;

        ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraPose> spPose;
        if (SUCCEEDED(hr))
        {
            hr = mHolographicNativeWindow->GetHolographicCameraPose(id, spPose.GetAddressOf());
        }

        if (SUCCEEDED(hr))
        {
            spPose->get_NearPlaneDistance(&mNearPlaneDistance);
            spPose->get_FarPlaneDistance(&mFarPlaneDistance);

            // Ensure the values we set previously made it through.
            assert(mNearPlaneDistance ==  0.1f);
            assert(mFarPlaneDistance  == 20.0f);

            ABI::Windows::Foundation::Rect viewportRect;
            spPose->get_Viewport(&viewportRect);
            mViewport = CD3D11_VIEWPORT(
                viewportRect.X,
                viewportRect.Y,
                viewportRect.Width,
                viewportRect.Height
            );
        }
    }
    
    HRESULT hr = S_OK;

    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters> spParameters;
    if (SUCCEEDED(hr))
    {
        // This will take care of back buffer, depth buffer, viewport, coordinate system, and view/projection matrix(es).
        hr = mHolographicNativeWindow->GetHolographicRenderingParameters(id, spParameters.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        EGLint result = updateHolographicRenderingParameters(spParameters);
            
        if (result != EGL_SUCCESS)
        {
            return result;
        }
        else
        {
            return EGL_SUCCESS;
        }
    }
    else
    {
        return EGL_BAD_DISPLAY;
    }
}

// parameters should be validated/clamped by caller
EGLint HolographicSwapChain11::swapRect(EGLint x, EGLint y, EGLint width, EGLint height)
{
    // Direct swap is not supported on Windows Holographic.
    return EGL_SUCCESS;
}

// parameters should be validated/clamped by caller
EGLint HolographicSwapChain11::swapRect(IHolographicFrame* pHolographicFrame)
{
    EGLint result = present(pHolographicFrame);
    mHolographicNativeWindow->ResetFrame();
    if (result != EGL_SUCCESS)
    {
        return result;
    }

    mRenderer->onSwap();

    return EGL_SUCCESS;
}

// This function uses the depth buffer for the first holographic camera to determine a best-
// fit plane for the scene geometry. It uses that plane, and a projected position, to set up
// the focus point and plane for Windows Holographic image stabilization. To take advantage 
// of this feature, make sure to apply the following rules when drawing content:
//   * All information in the depth buffer should be for hologram geometry that is visible
//     to the user.
//   * Don't draw pixels to the depth buffer to provide occlusion. Do occlusion last
//     instead; overwrite color pixels and turn off depth writes.
//   * Avoid rendering techniques that overwrite the depth buffer with other data.
void HolographicSwapChain11::SetStabilizationPlane(IHolographicFrame* pFrame)
{
    // Do the image stabilization plane estimation.

    using namespace Windows::Foundation::Numerics;

    // If the projection matrix isn't set yet, it will be the identity matrix.
    // In that case, we aren't ready to do depth-based image stabilization yet.
    if (mPreviousProjection.m43 == 0.f) return;

    mDepthBufferPlaneFinder->SetProjectionMatrix(mPreviousProjection);

    // Resolve depth for the current camera.
    float3 planeNormalInCameraSpace = {0, 0, 1.f};
    float distanceToPointInMeters = 2.0f - mNearPlaneDistance;
    assert(mDepthBufferPlaneFinder != nullptr);
    const bool estimationSuccess = mDepthBufferPlaneFinder->TryFindPlaneNormalAndDistance(
        this, 
        planeNormalInCameraSpace, 
        distanceToPointInMeters);

//#define ENABLE_DEBUG_OUTPUT
#ifdef ENABLE_DEBUG_OUTPUT
    OutputDebugStringA((
        "Distance from near plane to point: " + 
        std::to_string(distanceToPointInMeters) + 
        "\n").c_str());
#endif

    if (estimationSuccess)
    {
        // Transform the stabilization point, which is computed in view space, into the "world space" coordinate system.
        float3 pointInViewSpace = { 0.f, 0.f, -distanceToPointInMeters };
        float3 pointInWorldSpace = transform(pointInViewSpace, mPreviousViewInverse);

        // Note that the transpose of a rotation matrix is its inverse.
        float4x4 normalRotationInverse = transpose(mPreviousView);

        // Transform the LSR plane normal from view space to world space.
        planeNormalInCameraSpace = normalize(planeNormalInCameraSpace);
        float4 normalInCameraSpace = {
            planeNormalInCameraSpace.x,
           -planeNormalInCameraSpace.y,
            planeNormalInCameraSpace.z,
            dot(pointInViewSpace, planeNormalInCameraSpace)
            };
        float4 planeNormalInWorldSpace4 = transform(normalInCameraSpace, normalRotationInverse);
        float3 planeNormalInWorldSpace = {
            planeNormalInWorldSpace4.x,
            planeNormalInWorldSpace4.y,
            planeNormalInWorldSpace4.z
        };

        ComPtr<ABI::Windows::Perception::Spatial::ISpatialCoordinateSystem> spCoordinateSystem = 
            mHolographicNativeWindow->GetCoordinateSystem();

        // Set the focus point for the current camera.
        ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters> spParameters;
        if (SUCCEEDED(mHolographicNativeWindow->GetHolographicRenderingParameters(mHolographicCameraId, spParameters.GetAddressOf())))
        {
            spParameters->SetFocusPointWithNormal(
                spCoordinateSystem.Get(),
                pointInWorldSpace,
                planeNormalInWorldSpace);
        }
    }
}

EGLint HolographicSwapChain11::present(IHolographicFrame* pFrame)
{
    HRESULT result = S_OK;

    // For now, we set the focus plane for the first camera.
    if (mUseAutomaticDepthBasedImageStabilization)
    {
        SetStabilizationPlane(pFrame);

        mPreviousProjection    = mProjection;
        mPreviousView          = mView;
        mPreviousViewInverse   = mViewInverse;
    }

    // By default, this API waits for the frame to finish before it returns.
    // Holographic apps should wait for the previous frame to finish before
    // starting work on a new frame. This allows for better results from
    // holographic frame predictions.
    HolographicFramePresentResult presentResult;
        result = pFrame->PresentUsingCurrentPredictionWithBehavior(
            mWaitForVBlank ?
                HolographicFramePresentWaitBehavior_WaitForFrameToFinish :
                HolographicFramePresentWaitBehavior_DoNotWaitForFrameToFinish,
            &presentResult);
    
    if (SUCCEEDED(result))
    {
        // Some swapping mechanisms such as DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbind the current render
        // target.  Mark it dirty.
        mRenderer->markRenderTargetStateDirty();

        ComPtr<ID3D11DeviceContext1> spContext = mRenderer->getDeviceContext1IfSupported();

        // Discard the contents of the render target.
        // This is a valid operation only when the existing contents will be
        // entirely overwritten. If dirty or scroll rects are used, this call
        // should be removed.
        if (mBackBufferRTView != nullptr)
        {
            spContext->DiscardView(mBackBufferRTView.Get());
        }

        // Discard the contents of the depth stencil.
        if (mDepthStencilDSView != nullptr)
        {
            spContext->DiscardView(mDepthStencilDSView);
        }
    }

    // The PresentUsingCurrentPrediction API will detect when the graphics device
    // changes or becomes invalid. When this happens, it is considered a Direct3D
    // device lost scenario.
    if (presentResult == HolographicFramePresentResult::HolographicFramePresentResult_DeviceRemoved)
    {
        // The Direct3D device, context, and resources should be recreated.
        ERR("Present failed: the D3D11 device was removed: 0x%08X",
            mRenderer->getDevice()->GetDeviceRemovedReason());
        return EGL_CONTEXT_LOST;
    }

    else if (result == DXGI_ERROR_DEVICE_RESET)
    {
        ERR("Present failed: the D3D11 device was reset from a bad command.");
        return EGL_CONTEXT_LOST;
    }
    else if (FAILED(result))
    {
        ERR("Present failed with error code 0x%08X", result);
    }

    return EGL_SUCCESS;
}

ID3D11Texture2D *HolographicSwapChain11::getRenderTargetTexture()
{
    return mBackBufferTexture.Get();
}

ID3D11RenderTargetView *HolographicSwapChain11::getRenderTarget()
{
    // Note that the holographic render target view does not currently allow an offscreen texture.
    return mBackBufferRTView.Get();
}

ID3D11ShaderResourceView *HolographicSwapChain11::getRenderTargetShaderResource()
{
    // Note that the holographic render target view does not currently allow an offscreen texture.
    return mBackBufferSRView.Get();
}

ID3D11DepthStencilView *HolographicSwapChain11::getDepthStencil()
{
    return mDepthStencilDSView;
}

ID3D11ShaderResourceView * HolographicSwapChain11::getDepthStencilShaderResource()
{
    return mDepthStencilSRView;
}

ID3D11Texture2D *HolographicSwapChain11::getDepthStencilTexture()
{
    return mDepthStencilTexture;
}

DirectX::XMFLOAT4X4 const& HolographicSwapChain11::getMidViewMatrix()
{
    return mMidViewMatrix;
}

bool const& HolographicSwapChain11::getIsAutomaticStereoRenderingEnabled()
{
    return mUseAutomaticStereoRendering;
}

bool const& HolographicSwapChain11::getIsAutomaticDepthBasedImageStabilizationEnabled()
{
    return mUseAutomaticDepthBasedImageStabilization;
}

bool const& HolographicSwapChain11::getIsWaitForVBlankEnabled()
{
    return mWaitForVBlank;
}

void HolographicSwapChain11::setIsAutomaticStereoRenderingEnabled(bool isEnabled)
{
    mUseAutomaticStereoRendering = isEnabled;
}

void HolographicSwapChain11::setIsAutomaticDepthBasedImageStabilizationEnabled(bool isEnabled)
{
    mUseAutomaticDepthBasedImageStabilization = isEnabled;
}

void HolographicSwapChain11::setIsWaitForVBlankEnabled(bool isEnabled)
{
    mWaitForVBlank = isEnabled;
}

void HolographicSwapChain11::recreate()
{
    // possibly should use this method instead of reset
}

}
