//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceD3D.cpp: D3D implementation of an EGL surface

#include "libANGLE/renderer/d3d/SurfaceD3D.h"

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"
#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/renderer/d3d/SwapChainD3D.h"
#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#endif

#include <tchar.h>
#include <EGL/eglext.h>
#include <algorithm>

namespace rx
{

SurfaceD3D *SurfaceD3D::createOffscreen(RendererD3D *renderer, egl::Display *display, const egl::Config *config, EGLClientBuffer shareHandle,
                                        EGLint width, EGLint height)
{
    return new SurfaceD3D(renderer, display, config, width, height, EGL_TRUE, 0, EGL_FALSE,
                          shareHandle, NULL);
}

SurfaceD3D *SurfaceD3D::createFromWindow(RendererD3D *renderer,
                                         egl::Display *display,
                                         const egl::Config *config,
                                         EGLNativeWindowType window,
                                         EGLint fixedSize,
                                         EGLint directComposition,
                                         EGLint width,
                                         EGLint height,
                                         EGLint orientation)
{
    return new SurfaceD3D(renderer, display, config, width, height, fixedSize, orientation,
                          directComposition, static_cast<EGLClientBuffer>(0), window);
}

SurfaceD3D::SurfaceD3D(RendererD3D *renderer,
                       egl::Display *display,
                       const egl::Config *config,
                       EGLint width,
                       EGLint height,
                       EGLint fixedSize,
                       EGLint orientation,
                       EGLint directComposition,
                       EGLClientBuffer shareHandle,
                       EGLNativeWindowType window)
    : SurfaceImpl(),
      mRenderer(renderer),
      mDisplay(display),
      mFixedSize(fixedSize == EGL_TRUE),
      mOrientation(orientation),
      mRenderTargetFormat(config->renderTargetFormat),
      mDepthStencilFormat(config->depthStencilFormat),
      mSwapChain(nullptr),
      mSwapIntervalDirty(true),
      mNativeWindow(window, config, directComposition == EGL_TRUE),
      mWidth(width),
      mHeight(height),
      mSwapInterval(1),
      mShareHandle(reinterpret_cast<HANDLE *>(shareHandle))
{
}

SurfaceD3D::~SurfaceD3D()
{
    releaseSwapChain();
}

void SurfaceD3D::releaseSwapChain()
{
    // Holographic swap chains are owned by the HolographicNativeWindow, and will be deleted by it instead
    // when this is compiled for Windows Holographic.
#ifndef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    SafeDelete(mSwapChain);
#endif
}

egl::Error SurfaceD3D::initialize()
{
    if (mNativeWindow.getNativeWindow())
    {
        if (!mNativeWindow.initialize())
        {
            return egl::Error(EGL_BAD_SURFACE);
        }
    }

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    if (mNativeWindow.isHolographic())
    {
        // The Windows Holographic code path waits to create the swap chain, but it does need access to the D3D device at this point.
        // TODO: setd3ddevice on mNativeWindow, have it pass through
        HolographicNativeWindow* holographicNativeWindow = reinterpret_cast<HolographicNativeWindow*>(mNativeWindow.GetImpl());
        Renderer11* renderer11 = reinterpret_cast<Renderer11*>(mRenderer);
        holographicNativeWindow->setRenderer11(renderer11);

        ComPtr<ID3D11Device> device = reinterpret_cast<ID3D11Device*>(renderer11->getD3DDevice());
        holographicNativeWindow->setD3DDevice(device.Get());
    }
    else
#endif
    {
        egl::Error error = resetSwapChain();
        if (error.isError())
        {
            return error;
        }
    }

    return egl::Error(EGL_SUCCESS);
}

FramebufferImpl *SurfaceD3D::createDefaultFramebuffer(const gl::Framebuffer::Data &data)
{
    return mRenderer->createFramebuffer(data);
}

egl::Error SurfaceD3D::updateAttributeBoolean(EGLint attribute, EGLBoolean value)
{
    switch (attribute)
    {
    case EGLEXT_WAIT_FOR_VBLANK_ANGLE:
        if (mNativeWindow.isHolographic())
        {
            HolographicNativeWindow* holographicNativeWindow = reinterpret_cast<HolographicNativeWindow*>(mNativeWindow.GetImpl());
            return holographicNativeWindow->SetWaitForVBlank(static_cast<bool>(value));
        }
    default:
        return egl::Error(EGL_SUCCESS);
    }
}

egl::Error SurfaceD3D::updateAttributePointer(EGLint attribute, EGLNativeWindowType value)
{
    switch (attribute)
    {
    case EGLEXT_HOLOGRAPHIC_SPATIAL_FRAME_OF_REFERENCE_ANGLE:
        if (mNativeWindow.isHolographic())
        {
            HolographicNativeWindow* holographicNativeWindow = reinterpret_cast<HolographicNativeWindow*>(mNativeWindow.GetImpl());
            return holographicNativeWindow->SetSpatialFrameOfReference(value);
        }
    default:
        return egl::Error(EGL_SUCCESS);
    }
}

egl::Error SurfaceD3D::bindTexImage(gl::Texture *, EGLint)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::releaseTexImage(EGLint)
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::resetSwapChain()
{
    ASSERT(!mSwapChain);

    int width;
    int height;

    if (!mFixedSize)
    {
        RECT windowRect;
        if (!mNativeWindow.getClientRect(&windowRect))
        {
            ASSERT(false);

            return egl::Error(EGL_BAD_SURFACE, "Could not retrieve the window dimensions");
        }

        width = windowRect.right - windowRect.left;
        height = windowRect.bottom - windowRect.top;
    }
    else
    {
        // non-window surface - size is determined at creation
        width = mWidth;
        height = mHeight;
    }

    mSwapChain = mRenderer->createSwapChain(mNativeWindow, mShareHandle, mRenderTargetFormat,
        mDepthStencilFormat, mOrientation);
    if (!mSwapChain)
    {
        return egl::Error(EGL_BAD_ALLOC);
    }

    egl::Error error = resetSwapChain(width, height);
    if (error.isError())
    {
        SafeDelete(mSwapChain);
        return error;
    }

    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::resizeSwapChain(int backbufferWidth, int backbufferHeight)
{
    ASSERT(backbufferWidth >= 0 && backbufferHeight >= 0);
    ASSERT(mSwapChain);

    EGLint status = mSwapChain->resize(std::max(1, backbufferWidth), std::max(1, backbufferHeight));

    if (status == EGL_CONTEXT_LOST)
    {
        mDisplay->notifyDeviceLost();
        return egl::Error(status);
    }
    else if (status != EGL_SUCCESS)
    {
        return egl::Error(status);
    }

    mWidth = backbufferWidth;
    mHeight = backbufferHeight;

    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::resetSwapChain(int backbufferWidth, int backbufferHeight)
{
    EGLint status = EGL_SUCCESS;

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    // Holographic swap chains will have a back buffer width and height of 0
    // temporarily while waiting for the first holographic camera.
    if (!mNativeWindow.isHolographic())
#endif
    {
        ASSERT(backbufferWidth >= 0 && backbufferHeight >= 0);
        ASSERT(mSwapChain);
        mSwapChain->reset(std::max(1, backbufferWidth), std::max(1, backbufferHeight), mSwapInterval);
    }
#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    else
    {
        if (mSwapChain == nullptr)
        {
            // On Windows Holographic, we will have a null swap chain for a while
            // while waiting for the first holographic camera to arrive.
            // So, mSwapChain being null is expected.
            return egl::Error(EGL_SUCCESS);
        }

        // Update all holographic cameras.
        EGLint status = EGL_SUCCESS;
        for each (auto const& swapChain in mHolographicSwapChains)
        {
            EGLint s = swapChain->reset(std::max(1, backbufferWidth), std::max(1, backbufferHeight), mSwapInterval);
            if (s != EGL_SUCCESS)
            {
                status = s;
            }
        }
        if (status != EGL_SUCCESS)
        {
            // A reset is in progress. Don't try to draw to any holographic cameras this frame.
            mHolographicSwapChains.clear();
            mSwapChain = nullptr;
        }
    }
#endif

    if (status == EGL_CONTEXT_LOST)
    {
        mRenderer->notifyDeviceLost();
        return egl::Error(status);
    }
    else if (status != EGL_SUCCESS)
    {
        return egl::Error(status);
    }

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    if (mNativeWindow.isHolographic() && (mSwapChain != nullptr))
    {
        // Use the width and height of the holographic camera back buffer.
        HolographicSwapChain11* holographicSwapChain = reinterpret_cast<HolographicSwapChain11*>(mSwapChain);
        mWidth = holographicSwapChain->getWidth();
        mHeight = holographicSwapChain->getHeight();
    }
    else
#endif
    {
        mWidth = backbufferWidth;
        mHeight = backbufferHeight;
    }

    mSwapIntervalDirty = false;

    return egl::Error(EGL_SUCCESS);
}

egl::Error SurfaceD3D::swapRect(EGLint x, EGLint y, EGLint width, EGLint height)
{
    if (!mSwapChain)
    {
#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
        if (mNativeWindow.isHolographic())
        {
            // An error was encountered. In this case, we don't have a swap chain to 
            // present, and that is normally where the holographic native window would
            // be reset in preparation of acquiring a new holographic frame next time
            // the holographic FBO is cleared. So, we reset it here instead to ensure 
            // that the next frame is still going to be acquired.
            HolographicNativeWindow* holographicNativeWindow = reinterpret_cast<HolographicNativeWindow*>(mNativeWindow.GetImpl());
            holographicNativeWindow->ResetFrame();
        }
#endif
        return egl::Error(EGL_SUCCESS);
    }

    if (x + width > mWidth)
    {
        width = mWidth - x;
    }

    if (y + height > mHeight)
    {
        height = mHeight - y;
    }

    if (width != 0 && height != 0)
    {
        EGLint status = EGL_SUCCESS;

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
        if (mNativeWindow.isHolographic())
        {
            HolographicNativeWindow* holographicNativeWindow = reinterpret_cast<HolographicNativeWindow*>(mNativeWindow.GetImpl());
            auto frame = holographicNativeWindow->GetHolographicFrame();
            if (frame != nullptr)
            {
                HolographicSwapChain11* holographicSwapChain = reinterpret_cast<HolographicSwapChain11*>(mSwapChain);
                status = holographicSwapChain->swapRect(frame);
            }
        }
        else
#endif
        {
            status = mSwapChain->swapRect(x, y, width, height);
        }

        if (status == EGL_CONTEXT_LOST)
        {
            mRenderer->notifyDeviceLost();
            return egl::Error(status);
        }
        else if (status != EGL_SUCCESS)
        {
            return egl::Error(status);
        }
    }

    checkForOutOfDateSwapChain();

    return egl::Error(EGL_SUCCESS);
}

bool SurfaceD3D::checkForOutOfDateSwapChain()
{
    RECT client;
    int clientWidth = getWidth();
    int clientHeight = getHeight();
    bool sizeDirty = false;    
#ifndef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    if (!mFixedSize && !mNativeWindow.isIconic())
#else
    if (!mFixedSize && !mNativeWindow.isIconic() && !mNativeWindow.isHolographic())
#endif
    {
        // The window is automatically resized to 150x22 when it's minimized, but the swapchain shouldn't be resized
        // because that's not a useful size to render to.
        if (!mNativeWindow.getClientRect(&client))
        {
            ASSERT(false);
            return false;
        }

        // Grow the buffer now, if the window has grown. We need to grow now to avoid losing information.
        clientWidth = client.right - client.left;
        clientHeight = client.bottom - client.top;
        sizeDirty = clientWidth != getWidth() || clientHeight != getHeight();
    }

    bool wasDirty = (mSwapIntervalDirty || sizeDirty);

    if (mSwapIntervalDirty)
    {
        resetSwapChain(clientWidth, clientHeight);
    }
    else if (sizeDirty)
    {
        resizeSwapChain(clientWidth, clientHeight);
    }

    return wasDirty;
}

egl::Error SurfaceD3D::swap()
{
    return swapRect(0, 0, mWidth, mHeight);
}

egl::Error SurfaceD3D::postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height)
{
    return swapRect(x, y, width, height);
}

rx::SwapChainD3D *SurfaceD3D::getSwapChain() const
{
    return mSwapChain;
}

void SurfaceD3D::setSwapInterval(EGLint interval)
{
    if (mSwapInterval == interval)
    {
        return;
    }

    mSwapInterval = interval;
    mSwapIntervalDirty = true;
}

EGLint SurfaceD3D::getWidth() const
{
    return mWidth;
}

EGLint SurfaceD3D::getHeight() const
{
    return mHeight;
}

EGLint SurfaceD3D::isPostSubBufferSupported() const
{
    // post sub buffer is always possible on D3D surfaces
    return EGL_TRUE;
}

EGLint SurfaceD3D::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

egl::Error SurfaceD3D::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    if (attribute == EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE)
    {
        *value = mSwapChain->getShareHandle();
    }
    else if (attribute == EGL_DXGI_KEYED_MUTEX_ANGLE)
    {
        *value = mSwapChain->getKeyedMutex();
    }
    else UNREACHABLE();

    return egl::Error(EGL_SUCCESS);
}

gl::Error SurfaceD3D::getAttachmentRenderTarget(const gl::FramebufferAttachment::Target &target,
                                                FramebufferAttachmentRenderTarget **rtOut)
{
#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    // In holographic mode, the swap chain is provided by the system via a HolographicCamera.
    HRESULT hr = S_OK;
        
    // Create/upkeep the holographic swap chain
    if (mNativeWindow.isHolographic())
    {
        HolographicNativeWindow* holographicNativeWindow = reinterpret_cast<HolographicNativeWindow*>(mNativeWindow.GetImpl());

        // Update holographic resources - but only once per frame.
        if (holographicNativeWindow->IsUpdateNeeded())
        {
            HRESULT hrFromCameraUpdate = holographicNativeWindow->UpdateHolographicResources();

            // Check for changes to the list of holographic cameras.
            // We will clear and draw to each one.
            if (holographicNativeWindow->HasHolographicCameraListChanged() || (mSwapChain == nullptr))
            {
                mHolographicSwapChains.clear();
                mSwapChain = nullptr;
                auto cameraIds = holographicNativeWindow->GetCameraIds();
                int i = 0;
                for each (auto const& id in cameraIds)
                {
                    HolographicSwapChain11* swapChain = nullptr;
                    hr = holographicNativeWindow->GetHolographicSwapChain(id, &swapChain);
                    if (SUCCEEDED(hr))
                    {
                        if (i++ == 0)
                        {
                            // For now, we make the first holographic camera the "primary" 
                            // swap chain.
                            mSwapChain = swapChain;
                        }
                        mHolographicSwapChains.push_back(swapChain);
                    }
                }
            }

            resetSwapChain(0, 0);
        }
    }

    if (mSwapChain != nullptr)
#endif
    {
        if (target.binding() == GL_BACK)
        {
            *rtOut = mSwapChain->getColorRenderTarget();
        }
        else
        {
            *rtOut = mSwapChain->getDepthStencilRenderTarget();
        }
        return gl::Error(GL_NO_ERROR);
    }
#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
    else
    {
        return gl::Error(GL_INVALID_INDEX, "Failed to find a valid index into a holographic camera, HRESULT: 0x%X", hr);
    }
#endif
}

}
