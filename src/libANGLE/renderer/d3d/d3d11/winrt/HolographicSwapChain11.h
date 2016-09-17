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

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC

#include "angle_windowsstore.h"

#include "libANGLE/renderer/d3d/SwapChainD3D.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/winrt/HolographicNativeWindow.h"
#include "libANGLE/renderer/d3d/d3d11/winrt/DepthBufferPlaneFinder/ApplyDepthWeights.h"

#include <directxmath.h>
#include <memory>
#include <windows.graphics.directx.direct3d11.h>
#include <windows.graphics.holographic.h>
#include <windowsnumerics.h>

namespace HolographicDepthBasedImageStabilization
{
    class DepthBufferPlaneFinder;
}

namespace rx
{
class Renderer11;
class HolographicNativeWindow;

class HolographicSwapChain11 : public SwapChainD3D
{
  public:
      HolographicSwapChain11(
                Renderer11 *renderer,
                HolographicNativeWindow* nativeWindow,
                HANDLE shareHandle,
                ABI::Windows::Graphics::Holographic::IHolographicCamera* pHolographicCamera);
    virtual ~HolographicSwapChain11();

            EGLint  resize(EGLint backbufferWidth, EGLint backbufferHeight);
            EGLint  updateHolographicRenderingParameters(ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters>& cameraRenderingParameters);
    virtual EGLint  reset(EGLint backbufferWidth, EGLint backbufferHeight, EGLint swapInterval);
    virtual EGLint  swapRect(EGLint x, EGLint y, EGLint width, EGLint height);
            EGLint  swapRect(ABI::Windows::Graphics::Holographic::IHolographicFrame* pHolographicFrame);
    virtual void    recreate();

            RenderTargetD3D *           getColorRenderTarget()         override { return &mColorRenderTarget;          }
            RenderTargetD3D *           getDepthStencilRenderTarget()  override { return &mDepthStencilRenderTarget;   }

            ID3D11Texture2D *           getRenderTargetTexture();
    virtual ID3D11RenderTargetView *    getRenderTarget();
    virtual ID3D11ShaderResourceView *  getRenderTargetShaderResource();

    virtual ID3D11Texture2D *           getDepthStencilTexture();
    virtual ID3D11DepthStencilView *    getDepthStencil();
    virtual ID3D11ShaderResourceView *  getDepthStencilShaderResource();

    static DirectX::XMFLOAT4X4 const&   getMidViewMatrix();
    static bool const&                  getIsAutomaticStereoRenderingEnabled();
    static bool const&                  getIsAutomaticDepthBasedImageStabilizationEnabled();
    static bool const&                  getIsWaitForVBlankEnabled();
    static void                         setIsAutomaticStereoRenderingEnabled(bool isEnabled);
    static void                         setIsAutomaticDepthBasedImageStabilizationEnabled(bool isEnabled);
    static void                         setIsWaitForVBlankEnabled(bool isEnabled);

    HolographicNativeWindow *           getHolographicNativeWindow() { return mHolographicNativeWindow; }

    D3D11_VIEWPORT const& getViewport() const { return mViewport;           }
    DOUBLE  const& getNearPlane()       const { return mNearPlaneDistance;  }
    DOUBLE  const& getFarPlane()        const { return mFarPlaneDistance;   }
    boolean const& getIsStereo()        const { return mIsStereo;           }

    EGLint getWidth()                   const { return static_cast<EGLint>(mRenderTargetSize.Width);    }
    EGLint getHeight()                  const { return static_cast<EGLint>(mRenderTargetSize.Height);   }
    void * getKeyedMutex()           override { return mKeyedMutex;                                     }

    void release();
    
    ID3D11ShaderResourceView *  GetDepthShaderResourceView()            const { return mDepthStencilSRViewMono;         }
    ID3D11Buffer *              GetResolvedDepthTexture()               const { return mResolvedDepthBuffer;            }
    ID3D11Buffer *              GetCPUResolvedDepthTexture()            const { return mCPUResolvedDepthTexture;        }
    ID3D11Buffer *              GetMappableDefaultDepthTexture()        const { return mResolvedDepthBufferMappable;    }
    ID3D11UnorderedAccessView * GetResolvedDepthTextureView()           const { return mResolvedDepthView;              }
    ID3D11UnorderedAccessView * GetResolvedDepthTextureViewMappable()   const { return mResolvedDepthViewMappable;      }
    
    Windows::Foundation::Numerics::float4x4 const& GetViewMatrix()          const { return mView;          }
    Windows::Foundation::Numerics::float4x4 const& GetInverseViewMatrix()   const { return mViewInverse;   }
    Windows::Foundation::Numerics::float4x4 const& GetProjectionMatrix()    const { return mProjection;    }

    float const& GetNearPlane()         const { return mNearPlaneDistance; }
    float const& GetFarPlane()          const { return mFarPlaneDistance;  }
    float const* GetDepthWeightArray()  const { return mDepthWeightArray;  }

  private:

    EGLint      resetOffscreenBuffers(
                    int backbufferWidth, 
                    int backbufferHeight);
    void        releaseOffscreenDepthBuffer();
    EGLint      resetOffscreenDepthBuffer(
                    int backbufferWidth, 
                    int backbufferHeight);
    DXGI_FORMAT getSwapChainNativeFormat() const;
    EGLint      present(
                    ABI::Windows::Graphics::Holographic::IHolographicFrame* pFrame);
    void        SetStabilizationPlane(
                    ABI::Windows::Graphics::Holographic::IHolographicFrame* pFrame);
    void        ComputeMidViewMatrix(
                    const DirectX::XMMATRIX& left,
                    const DirectX::XMMATRIX& right);

    // Windows Holographic camera 
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCamera> mHolographicCamera = nullptr;

    // Automatic depth-based focus plane.
    std::unique_ptr<HolographicDepthBasedImageStabilization::DepthBufferPlaneFinder> mDepthBufferPlaneFinder;

    rx::HolographicNativeWindow        *mHolographicNativeWindow;

    Renderer11                         *mRenderer;
    IDXGIKeyedMutex                    *mKeyedMutex;

    ComPtr<ID3D11Texture2D>             mBackBufferTexture;
    ComPtr<ID3D11RenderTargetView>      mBackBufferRTView;
    ComPtr<ID3D11ShaderResourceView>    mBackBufferSRView;

    ID3D11Texture2D                    *mDepthStencilTexture;
    ID3D11DepthStencilView             *mDepthStencilDSView;
    ID3D11ShaderResourceView           *mDepthStencilSRView;

    SurfaceRenderTarget11               mColorRenderTarget;
    SurfaceRenderTarget11               mDepthStencilRenderTarget;
    
    ID3D11ShaderResourceView           *mDepthStencilSRViewMono;
    ID3D11Buffer                       *mResolvedDepthBuffer;
    ID3D11Buffer                       *mCPUResolvedDepthTexture;
    ID3D11UnorderedAccessView          *mResolvedDepthView;
    ID3D11Buffer                       *mResolvedDepthBufferMappable;
    ID3D11UnorderedAccessView          *mResolvedDepthViewMappable;
    
    // Holographic camera ID
    UINT32                              mHolographicCameraId;

    // Direct3D rendering properties.
    DXGI_FORMAT                         mDxgiFormat;
    D3D11_VIEWPORT                      mViewport;
    ABI::Windows::Foundation::Size      mRenderTargetSize;
    DOUBLE                              mViewportScaleFactor;

    // Indicates whether the camera supports stereoscopic rendering.
    boolean                             mIsStereo = false;

    // Indicates whether this camera has a pending frame.
    bool                                mFramePending = false;
    
    // A mono view matrix that represents the holographic camera's position and 
    // orientation within the coordinate system provided by the app.
    static DirectX::XMFLOAT4X4          mMidViewMatrix;
    static DirectX::XMFLOAT4X4          mMidViewMatrixInverse;

    // Tracks whether or not the app has enabled automatic stereo instancing.
    static bool                         mUseAutomaticStereoRendering;

    // Tracks whether or not the app has enabled automatic depth-based image stabilization.
    static bool                         mUseAutomaticDepthBasedImageStabilization;

    // Whether or not the present call blocks until vblank has occurred.
    static bool                         mWaitForVBlank;

    // Variables used with automatic depth-based image stabilization

    // The precomputed array of depth weights that are used for this camera.
    float mDepthWeightArray[DEPTH_ARRAY_SIZE];
        
    // The near and far plane.
    DOUBLE mNearPlaneDistance =  0.1f;
    DOUBLE mFarPlaneDistance  = 20.0f;
    
    // Double-buffered view/projection matrices.
    Windows::Foundation::Numerics::float4x4 mView                   = Windows::Foundation::Numerics::float4x4::identity();
    Windows::Foundation::Numerics::float4x4 mViewInverse            = Windows::Foundation::Numerics::float4x4::identity();
    Windows::Foundation::Numerics::float4x4 mProjection             = Windows::Foundation::Numerics::float4x4::identity();
    Windows::Foundation::Numerics::float4x4 mPreviousView           = Windows::Foundation::Numerics::float4x4::identity();
    Windows::Foundation::Numerics::float4x4 mPreviousViewInverse    = Windows::Foundation::Numerics::float4x4::identity();
    Windows::Foundation::Numerics::float4x4 mPreviousProjection     = Windows::Foundation::Numerics::float4x4::identity();
};

}
#else // ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
namespace rx
{
class HolographicSwapChain11
{
public:
    static bool getIsAutomaticStereoRenderingEnabled() { return false; };
    static bool getIsAutomaticDepthBasedImageStabilizationEnabled() { return false; };
};
}
#endif // ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
#endif // LIBANGLE_RENDERER_D3D_D3D11_SWAPCHAIN11_H_
