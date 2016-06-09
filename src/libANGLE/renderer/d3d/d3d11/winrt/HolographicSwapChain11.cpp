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

HolographicSwapChain11::HolographicSwapChain11(Renderer11 *renderer,
                         HolographicNativeWindow* nativeWindow,
                         HANDLE shareHandle,
                         ABI::Windows::Graphics::Holographic::IHolographicCamera* pCamera)
    : SwapChainD3D(*((NativeWindow*)nativeWindow), shareHandle, GL_RGBA, GL_DEPTH_COMPONENT16),
      mHolographicNativeWindow(nativeWindow),
      mRenderer(renderer),
      m_holographicCamera(pCamera),
      mBackBufferTexture(nullptr),
      mBackBufferRTView(nullptr),
      mBackBufferSRView(nullptr),
      mDepthStencilTexture(nullptr),
      mDepthStencilDSView(nullptr),
      mDepthStencilSRView(nullptr),
      mColorRenderTarget(this, renderer, false),
      mDepthStencilRenderTarget(this, renderer, true)
{
    pCamera->get_RenderTargetSize(&mD3DRenderTargetSize);
    pCamera->get_ViewportScaleFactor(&mD3DViewportScaleFactor);
    pCamera->get_IsStereo(&mIsStereo);

    // cache the ID
    m_holographicCamera->get_Id(&mId);
}

HolographicSwapChain11::~HolographicSwapChain11()
{
    release();
}

void HolographicSwapChain11::release()
{
    SafeRelease(mKeyedMutex);
    SafeRelease(mBackBufferTexture);
    SafeRelease(mBackBufferRTView);
    SafeRelease(mBackBufferSRView);
    SafeRelease(mDepthStencilTexture);
    SafeRelease(mDepthStencilDSView);
    SafeRelease(mDepthStencilSRView);
}

void HolographicSwapChain11::releaseOffscreenDepthBuffer()
{
    SafeRelease(mDepthStencilTexture);
    SafeRelease(mDepthStencilDSView);
    SafeRelease(mDepthStencilSRView);
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

    if (mDepthBufferFormat != GL_NONE)
    {
        const d3d11::TextureFormat &depthBufferFormatInfo =
            d3d11::GetTextureFormatInfo(mDepthBufferFormat, mRenderer->getRenderer11DeviceCaps());

        D3D11_TEXTURE2D_DESC depthStencilTextureDesc;
        depthStencilTextureDesc.Width = backbufferWidth;
        depthStencilTextureDesc.Height = backbufferHeight;
        depthStencilTextureDesc.Format = depthBufferFormatInfo.texFormat;
        depthStencilTextureDesc.MipLevels = 1;
        depthStencilTextureDesc.ArraySize = mIsStereo ? 2 : 1;
        depthStencilTextureDesc.SampleDesc.Count = 1;
        depthStencilTextureDesc.SampleDesc.Quality = 0;
        depthStencilTextureDesc.Usage = D3D11_USAGE_DEFAULT;
        depthStencilTextureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

        if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
        {
            depthStencilTextureDesc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
        }

        depthStencilTextureDesc.CPUAccessFlags = 0;
        depthStencilTextureDesc.MiscFlags = 0;

        ID3D11Device *device = mRenderer->getDevice();
        HRESULT result =
            device->CreateTexture2D(&depthStencilTextureDesc, NULL, &mDepthStencilTexture);
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

        D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc;
        depthStencilDesc.Format = depthBufferFormatInfo.dsvFormat;
        depthStencilDesc.ViewDimension = mIsStereo ? D3D11_DSV_DIMENSION_TEXTURE2DARRAY : D3D11_DSV_DIMENSION_TEXTURE2D;
        depthStencilDesc.Flags = 0;
        depthStencilDesc.Texture2D.MipSlice = 0;
        depthStencilDesc.Texture2DArray.ArraySize = 2;
        depthStencilDesc.Texture2DArray.FirstArraySlice = 0;
        depthStencilDesc.Texture2DArray.MipSlice = 0;

        result = device->CreateDepthStencilView(mDepthStencilTexture, &depthStencilDesc, &mDepthStencilDSView);
        ASSERT(SUCCEEDED(result));
        d3d11::SetDebugName(mDepthStencilDSView, "Offscreen depth stencil view");

        if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
        {
            D3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc;
            depthStencilSRVDesc.Format = depthBufferFormatInfo.srvFormat;
            depthStencilSRVDesc.ViewDimension = mIsStereo ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D;
            depthStencilSRVDesc.Texture2D.MostDetailedMip = 0;
            depthStencilSRVDesc.Texture2D.MipLevels = static_cast<UINT>(-1);
            depthStencilSRVDesc.Texture2DArray.ArraySize = 2;
            depthStencilSRVDesc.Texture2DArray.FirstArraySlice = 0;
            depthStencilSRVDesc.Texture2DArray.MostDetailedMip = 0;
            depthStencilSRVDesc.Texture2DArray.MipLevels = static_cast<UINT>(-1);

            result = device->CreateShaderResourceView(mDepthStencilTexture, &depthStencilSRVDesc, &mDepthStencilSRView);
            ASSERT(SUCCEEDED(result));
            d3d11::SetDebugName(mDepthStencilSRView, "Offscreen depth stencil shader resource");
        }

        if (mIsStereo)
        {
            D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc2;
            depthStencilDesc2.Format = depthBufferFormatInfo.dsvFormat;
            depthStencilDesc2.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DARRAY;
            depthStencilDesc2.Flags = 0;
            depthStencilDesc2.Texture2DArray.ArraySize = 1;
            depthStencilDesc2.Texture2DArray.FirstArraySlice = 1;
            depthStencilDesc2.Texture2DArray.MipSlice = 0;

            result = device->CreateDepthStencilView(mDepthStencilTexture, &depthStencilDesc2, &mDepthStencilDSView2);
            ASSERT(SUCCEEDED(result));
            d3d11::SetDebugName(mDepthStencilDSView, "Offscreen depth stencil view");

            if (depthBufferFormatInfo.srvFormat != DXGI_FORMAT_UNKNOWN)
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC depthStencilSRVDesc;
                depthStencilSRVDesc.Format = depthBufferFormatInfo.srvFormat;
                depthStencilSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                depthStencilSRVDesc.Texture2DArray.ArraySize = 1;
                depthStencilSRVDesc.Texture2DArray.FirstArraySlice = 1;
                depthStencilSRVDesc.Texture2DArray.MostDetailedMip = 0;
                depthStencilSRVDesc.Texture2DArray.MipLevels = static_cast<UINT>(-1);

                result = device->CreateShaderResourceView(mDepthStencilTexture, &depthStencilSRVDesc, &mDepthStencilSRView2);
                ASSERT(SUCCEEDED(result));
                d3d11::SetDebugName(mDepthStencilSRView, "Offscreen depth stencil shader resource");
            }
        }
    }

    return EGL_SUCCESS;
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

EGLint HolographicSwapChain11::updateHolographicRenderingParameters(
    ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters* pCameraRenderingParameters)
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

    // Get the WinRT object representing the holographic camera's back buffer.
    ComPtr<IDirect3DSurface> surface;
    if (SUCCEEDED(result))
    {
        result = pCameraRenderingParameters->get_Direct3D11BackBuffer(surface.GetAddressOf());
    }

    // Get a DXGI interface for the holographic camera's back buffer.
    // Holographic cameras do not provide the DXGI swap chain, which is owned
    // by the system. The Direct3D back buffer resource is provided using WinRT
    // interop APIs.
    ComPtr<IInspectable> inspectable;
    if (SUCCEEDED(result))
    {
        result = surface.As(&inspectable);
    }
    ComPtr<IDirect3DDxgiInterfaceAccess> dxgiInterfaceAccess;
    if (SUCCEEDED(result))
    {
        result = inspectable.As(&dxgiInterfaceAccess);
    }
    ComPtr<ID3D11Resource> resource;
    if (SUCCEEDED(result))
    {
        result = dxgiInterfaceAccess->GetInterface(IID_PPV_ARGS(&resource));
    }
    // Get a Direct3D interface for the holographic camera's back buffer.
    ComPtr<ID3D11Texture2D> cameraBackBuffer;
    if (SUCCEEDED(result))
    {
        result = resource.As(&cameraBackBuffer);
    }

    // Don't resize unnecessarily
    if (mBackBufferTexture != cameraBackBuffer.Get())
    {
        // Can only recreate views if we have a resource
        ASSERT(cameraBackBuffer);

        SafeRelease(mBackBufferTexture);
        SafeRelease(mBackBufferRTView);
        SafeRelease(mBackBufferSRView);

        // This can change every frame as the system moves to the next buffer in the
        // swap chain. This mode of operation will occur when certain rendering modes
        // are activated.
        mBackBufferTexture = cameraBackBuffer.Get();

        // Create a render target view of the back buffer.
        // Creating this resource is inexpensive, and is better than keeping track of
        // the back buffers in order to pre-allocate render target views for each one.
        d3d11::SetDebugName(mBackBufferTexture, "Back buffer texture");
        result = device->CreateRenderTargetView(mBackBufferTexture, NULL, &mBackBufferRTView);
        ASSERT(SUCCEEDED(result));
        if (SUCCEEDED(result))
        {
            d3d11::SetDebugName(mBackBufferRTView, "Back buffer render target");
        }

        // Get the DXGI format for the back buffer.
        // This information can be accessed by the app using CameraResources::GetBackBufferDXGIFormat().
        D3D11_TEXTURE2D_DESC backBufferDesc;
        mBackBufferTexture->GetDesc(&backBufferDesc);
        m_dxgiFormat = backBufferDesc.Format;

        // Check for render target size changes.
        ABI::Windows::Foundation::Size currentSize;
        result = m_holographicCamera->get_RenderTargetSize(&currentSize);
        if (SUCCEEDED(result))
        {
            if (mD3DRenderTargetSize.Height != currentSize.Height ||
                mD3DRenderTargetSize.Width != currentSize.Width)
            {
                // Set render target size.
                mD3DRenderTargetSize = currentSize;
            }
        }

        if (SUCCEEDED(result))
        {
            result = device->CreateRenderTargetView(
                mBackBufferTexture,
                nullptr,
                &mBackBufferRTView
            );
        }

        if (SUCCEEDED(result))
        {
            CD3D11_SHADER_RESOURCE_VIEW_DESC desc(
                mIsStereo ? D3D11_SRV_DIMENSION_TEXTURE2DARRAY : D3D11_SRV_DIMENSION_TEXTURE2D,
                backBufferDesc.Format);
            HRESULT ignoreThisResult = device->CreateShaderResourceView(mBackBufferTexture, &desc, &mBackBufferSRView);

            if (SUCCEEDED(ignoreThisResult))
            {
                d3d11::SetDebugName(mBackBufferSRView, "Back buffer shader resource");
            }
            else
            {
                mBackBufferSRView = nullptr;
            }
        }

        if (mIsStereo)
        {
            if (SUCCEEDED(result))
            {
                D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
                renderTargetDesc.Format = backBufferDesc.Format;
                renderTargetDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                renderTargetDesc.Texture2DArray.ArraySize = 1;
                renderTargetDesc.Texture2DArray.FirstArraySlice = 1;
                renderTargetDesc.Texture2DArray.MipSlice = 0;

                result = device->CreateRenderTargetView(
                    mBackBufferTexture,
                    &renderTargetDesc,
                    &mBackBufferRTView2
                );
            }

            if (SUCCEEDED(result))
            {
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
                srvDesc.Format = backBufferDesc.Format;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
                srvDesc.Texture2DArray.ArraySize = 1;
                srvDesc.Texture2DArray.FirstArraySlice = 1;
                srvDesc.Texture2DArray.MipLevels = 0;
                srvDesc.Texture2DArray.MostDetailedMip = 0;

                HRESULT ignoreThisResult = device->CreateShaderResourceView(mBackBufferTexture, &srvDesc, &mBackBufferSRView2);

                if (SUCCEEDED(ignoreThisResult))
                {
                    d3d11::SetDebugName(mBackBufferSRView, "Back buffer shader resource");
                }
                else
                {
                    mBackBufferSRView2 = nullptr;
                }
            }
        }

        // A new depth stencil view is also needed.
        return resetOffscreenBuffers(
            static_cast<EGLint>(mD3DRenderTargetSize.Width),
            static_cast<EGLint>(mD3DRenderTargetSize.Height));
    }

    if (SUCCEEDED(result))
    {
        static ComPtr<ABI::Windows::Perception::Spatial::ISpatialCoordinateSystem> coordinateSystem = mHolographicNativeWindow->GetCoordinateSystem();

        if (coordinateSystem != nullptr)
        {
            ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraPose> pose;
            result = mHolographicNativeWindow->GetHolographicCameraPoses()->GetAt(mId, pose.GetAddressOf());

            ABI::Windows::Graphics::Holographic::HolographicStereoTransform viewTransform;
            if (SUCCEEDED(result))
            {
                ComPtr<IReference<ABI::Windows::Graphics::Holographic::HolographicStereoTransform>> viewTransformReference;
                result = pose->TryGetViewTransform(coordinateSystem.Get(), viewTransformReference.GetAddressOf());

                if (SUCCEEDED(result) && (viewTransformReference != nullptr))
                {
                    result = viewTransformReference->get_Value(&viewTransform);
                }
            }

            ABI::Windows::Graphics::Holographic::HolographicStereoTransform projectionTransform;
            if (SUCCEEDED(result))
            {
                result = pose->get_ProjectionTransform(&projectionTransform);
            }

            static DirectX::XMFLOAT4X4 viewProj[2];
            gl::Program *program = nullptr;
            gl::Context *context = nullptr;
            if (SUCCEEDED(result))
            {
                // Update the view matrices. Holographic cameras (such as Microsoft HoloLens) are
                // constantly moving relative to the world. The view matrices need to be updated
                // every frame.
                DirectX::XMStoreFloat4x4(
                    &viewProj[0],
                    /*DirectX::XMMatrixTranspose*/(
                        DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Left)) * DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Left)))
                );
                DirectX::XMStoreFloat4x4(
                    &viewProj[1],
                    /*DirectX::XMMatrixTranspose*/(
                        DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&viewTransform.Right)) * DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&projectionTransform.Right)))
                );

                // get the current program
                context = gl::GetValidGlobalContext();
                program = context->getState().getProgram();

                if (program)
                {
                    GLint viewProjUniformLocation = -1;
                    if (SUCCEEDED(result))
                    {
                        viewProjUniformLocation = program->getUniformLocation("uHolographicViewProjectionMatrix");

                        if (viewProjUniformLocation == -1)
                        {
                            result = E_FAIL;
                        }
                    }

                    if (SUCCEEDED(result))
                    {
                        // detach any existing buffers
                        context->bindGenericUniformBuffer(0);

                        if (!(gl::ValidateUniformMatrix(context, GL_FLOAT_MAT4, viewProjUniformLocation, 2, GL_FALSE)))
                        {
                            result = E_FAIL;
                        }
                        program->setUniformMatrix4fv(viewProjUniformLocation, 2, GL_FALSE, (GLfloat*) viewProj);
                    }
                }
            }
        }
        else
        {
            // TODO: Handle error scenario
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
    return m_dxgiFormat;
}

EGLint HolographicSwapChain11::reset(int backbufferWidth, int backbufferHeight, EGLint swapInterval)
{
    // Windows Holographic apps use APIs to access the back buffer.
    UINT32 id = 0;
    HRESULT hr = m_holographicCamera->get_Id(&id);

    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraPose> pose;
    if (SUCCEEDED(hr))
    {
        hr = mHolographicNativeWindow->GetHolographicCameraPose(id, pose.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        pose->get_NearPlaneDistance(&mNearPlaneDistance);
        pose->get_FarPlaneDistance(&mFarPlaneDistance);

        ABI::Windows::Foundation::Rect viewportRect;
        pose->get_Viewport(&viewportRect);
        m_d3dViewport = CD3D11_VIEWPORT(
            viewportRect.X,
            viewportRect.Y,
            viewportRect.Width,
            viewportRect.Height
        );
    }

    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters> parameters;
    if (SUCCEEDED(hr))
    {
        // This will take care of back buffer, depth buffer, viewport, coordinate system, and view/projection matrix(es).
        hr = mHolographicNativeWindow->GetHolographicRenderingParameters(id, parameters.GetAddressOf());
    }

    if (SUCCEEDED(hr))
    {
        if (mParameters != parameters)
        {
            mParameters = parameters;
        }
        else
        {
            return EGL_SUCCESS;
        }

        EGLint result = updateHolographicRenderingParameters(parameters.Get());

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
    if (result != EGL_SUCCESS)
    {
        return result;
    }

    mRenderer->onSwap();

    return EGL_SUCCESS;
}

EGLint HolographicSwapChain11::present(IHolographicFrame* pFrame)
{
    HRESULT result = S_OK;

    // By default, this API waits for the frame to finish before it returns.
    // Holographic apps should wait for the previous frame to finish before
    // starting work on a new frame. This allows for better results from
    // holographic frame predictions.
    HolographicFramePresentResult presentResult;
    result = pFrame->PresentUsingCurrentPrediction(&presentResult);
    
    if (SUCCEEDED(result))
    {
        // Some swapping mechanisms such as DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL unbind the current render
        // target.  Mark it dirty.
        mRenderer->markRenderTargetStateDirty();

        //ComPtr<IHolographicFramePrediction> prediction;
        //result = pFrame->get_CurrentPrediction(prediction.GetAddressOf());
        //UseHolographicCameraResources<void>([this, prediction](std::map<UINT32, std::unique_ptr<CameraResources>>& cameraResourceMap)
        //{
            //for (auto cameraPose : prediction->CameraPoses)
            //{
                ComPtr<ID3D11DeviceContext1> spContext = mRenderer->getDeviceContext1IfSupported();

                // This represents the device-based resources for a HolographicCamera.
                //DX::CameraResources* pCameraResources = cameraResourceMap[cameraPose->HolographicCamera->Id].get();

                // Discard the contents of the render target.
                // This is a valid operation only when the existing contents will be
                // entirely overwritten. If dirty or scroll rects are used, this call
                // should be removed.
                //spContext->DiscardView(pCameraResources->GetBackBufferRenderTargetView());
                if (mBackBufferRTView != nullptr)
                {
                    spContext->DiscardView(mBackBufferRTView);
                }

                // Discard the contents of the depth stencil.
                if (mDepthStencilDSView != nullptr)
                {
                    spContext->DiscardView(mDepthStencilDSView);
                }
        //}
    //});
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
    return mBackBufferTexture;
}

ID3D11RenderTargetView *HolographicSwapChain11::getRenderTarget()
{
    if (mBackBufferRTView == nullptr)
    {
        // TODO: Use the latest holographic rendering parameters to ensure a back buffer exists.
        //       We might need a global with which to do that.
    }
    return /*mNeedsOffscreenTexture ? mOffscreenRTView : */mBackBufferRTView;
}

ID3D11ShaderResourceView *HolographicSwapChain11::getRenderTargetShaderResource()
{
    return /*mNeedsOffscreenTexture ? mOffscreenSRView : */mBackBufferSRView;
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

ID3D11RenderTargetView *HolographicSwapChain11::getRenderTarget2()
{
    if (mBackBufferRTView2 == nullptr)
    {
        // TODO: use the latest holographic rendering parameters to ensure a back buffer exists
        // we might need a global with which to do that
    }
    return /*mNeedsOffscreenTexture ? mOffscreenRTView : */mBackBufferRTView2;
}

ID3D11ShaderResourceView *HolographicSwapChain11::getRenderTargetShaderResource2()
{
    return /*mNeedsOffscreenTexture ? mOffscreenSRView : */mBackBufferSRView2;
}

ID3D11DepthStencilView *HolographicSwapChain11::getDepthStencil2()
{
    return mDepthStencilDSView2;
}

ID3D11ShaderResourceView * HolographicSwapChain11::getDepthStencilShaderResource2()
{
    return mDepthStencilSRView2;
}

void HolographicSwapChain11::recreate()
{
    // possibly should use this method instead of reset
}

}
