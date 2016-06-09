//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HolographicSwapChain11.h: Defines a back-end specific class for the D3D11 swap chain, which is based on a holographic camera.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_HOLOGRAPHICSWAPCHAIN11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_HOLOGRAPHICSWAPCHAIN11_H_


#include "common/angleutils.h"
#include "common/platform.h"

#include "angle_windowsstore.h"

#include "libANGLE/renderer/d3d/SwapChainD3D.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/winrt/HolographicNativeWindow.h"

#include <windows.graphics.directx.direct3d11.h>
#include <windows.graphics.holographic.h>

namespace rx
{
class Renderer11;
class HolographicNativeWindow;

class HolographicSwapChain11 : public SwapChainD3D
{
  public:
      HolographicSwapChain11(Renderer11 *renderer,
                HolographicNativeWindow* nativeWindow,
                HANDLE shareHandle,
                ABI::Windows::Graphics::Holographic::IHolographicCamera* pHolographicCamera);
    virtual ~HolographicSwapChain11();

    EGLint resize(EGLint backbufferWidth, EGLint backbufferHeight);
    EGLint updateHolographicRenderingParameters(ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters* pCameraRenderingParameters);
    virtual EGLint reset(EGLint backbufferWidth, EGLint backbufferHeight, EGLint swapInterval);
    virtual EGLint swapRect(EGLint x, EGLint y, EGLint width, EGLint height);
            EGLint swapRect(ABI::Windows::Graphics::Holographic::IHolographicFrame* pHolographicFrame);
    virtual void recreate();

    RenderTargetD3D *getColorRenderTarget() override { return &mColorRenderTarget; }
    RenderTargetD3D *getDepthStencilRenderTarget() override { return &mDepthStencilRenderTarget; }

            ID3D11Texture2D *getRenderTargetTexture();
    virtual ID3D11RenderTargetView *getRenderTarget();
    virtual ID3D11ShaderResourceView *getRenderTargetShaderResource();

    virtual ID3D11Texture2D *getDepthStencilTexture();
    virtual ID3D11DepthStencilView *getDepthStencil();
    virtual ID3D11ShaderResourceView *getDepthStencilShaderResource();

    virtual ID3D11RenderTargetView *getRenderTarget2();
    virtual ID3D11ShaderResourceView *getRenderTargetShaderResource2();

    virtual ID3D11DepthStencilView *getDepthStencil2();
    virtual ID3D11ShaderResourceView *getDepthStencilShaderResource2();

    HolographicNativeWindow* getHolographicNativeWindow() { return mHolographicNativeWindow; }

    D3D11_VIEWPORT const& getViewport() const { return m_d3dViewport;       }
    DOUBLE const& getNearPlane()        const { return mNearPlaneDistance;  }
    DOUBLE const& getFarPlane()         const { return mFarPlaneDistance;   }
    boolean const& getIsStereo()        const { return mIsStereo;          }

    EGLint getWidth()                   const { return static_cast<EGLint>(mD3DRenderTargetSize.Width);    }
    EGLint getHeight()                  const { return static_cast<EGLint>(mD3DRenderTargetSize.Height);   }
    void *getKeyedMutex()            override { return mKeyedMutex;                                         }

    void release();

  private:

    EGLint resetOffscreenBuffers(int backbufferWidth, int backbufferHeight);
    void releaseOffscreenDepthBuffer();
    EGLint resetOffscreenDepthBuffer(int backbufferWidth, int backbufferHeight);

    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters> mParameters = nullptr;

    DXGI_FORMAT getSwapChainNativeFormat() const;

    EGLint present(ABI::Windows::Graphics::Holographic::IHolographicFrame* pFrame);

    Renderer11                  *mRenderer;
    IDXGIKeyedMutex             *mKeyedMutex;

    ID3D11Texture2D             *mBackBufferTexture;
    ID3D11RenderTargetView      *mBackBufferRTView;
    ID3D11ShaderResourceView    *mBackBufferSRView;

    ID3D11Texture2D             *mDepthStencilTexture;
    ID3D11DepthStencilView      *mDepthStencilDSView;
    ID3D11ShaderResourceView    *mDepthStencilSRView;

    SurfaceRenderTarget11        mColorRenderTarget;
    SurfaceRenderTarget11        mDepthStencilRenderTarget;

    // For rendering in stereo without using instancing. 
    // Not currently used.
    ID3D11RenderTargetView      *mBackBufferRTView2;
    ID3D11ShaderResourceView    *mBackBufferSRView2;
    ID3D11DepthStencilView      *mDepthStencilDSView2;
    ID3D11ShaderResourceView    *mDepthStencilSRView2;

    /*SurfaceRenderTarget11 mColorRenderTarget2;
    SurfaceRenderTarget11 mDepthStencilRenderTarget2;*/

    rx::HolographicNativeWindow *mHolographicNativeWindow;

    // Windows Holographic camera parameters

    Microsoft::WRL::ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCamera> m_holographicCamera;

    // Device resource to store view and projection matrices.
    Microsoft::WRL::ComPtr<ID3D11Buffer>                m_viewProjectionConstantBuffer;

    // Direct3D rendering properties.
    DXGI_FORMAT                                         m_dxgiFormat;
    D3D11_VIEWPORT                                      m_d3dViewport;
    ABI::Windows::Foundation::Size                      mD3DRenderTargetSize;
    DOUBLE                                              mD3DViewportScaleFactor;
    DOUBLE                                              mNearPlaneDistance;
    DOUBLE                                              mFarPlaneDistance;

    // Indicates whether the camera supports stereoscopic rendering.
    boolean                                             mIsStereo = false;

    // Holographic camera ID.
    UINT32 mId = 0;

    // Indicates whether this camera has a pending frame.
    bool                                                m_framePending = false;

    // Pointer to the holographic camera these resources are for.
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCamera> m_spHolographicCamera = nullptr;
};

}
#endif // LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_
