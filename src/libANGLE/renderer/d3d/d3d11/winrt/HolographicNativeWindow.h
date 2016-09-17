//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HolographicNativeWindow.h: NativeWindow for managing windows based on Windows Holographic app views.
//
// This implementation uses the frame of reference provided by the app to render a stereo version of 
// the scene. It can provide a view matrix midway between both eye positions of the holographic camera 
// for the app to use, and in this case the app is expected to use a projection matrix that is the 
// identity matrix (or no projection matrix at all). Internally to ANGLE, the stereo rendering will 
// double the number of instances provided to the shader - which is hidden from the shader by halving 
// the value set for gl_InstanceID - and apply the left and right view matrices to each eye, after 
// undoing the midview matrix.
//
// Alternatively, the app can use its own instanced draw calls and access the holographic view/projection 
// matrix provided by ANGLE by using odd and even instance IDs for left and right views. In this case, the 
// shader program is not modified.
//
// This implementation can also use automatic depth-based image stabilization. This feature determines a 
// best-fit plane for the scene geometry by processing geometry data from the depth buffer, and then sets 
// the focus point and plane for Windows Holographic image stabilization. To take advantage of this 
// feature, make sure to apply the following rules when drawing content:
//   * All information in the depth buffer should be for hologram geometry that is visible
//     to the user.
//   * Don't draw pixels to the depth buffer to provide occlusion. Do occlusion last
//     instead; overwrite color pixels and turn off depth writes.
//   * Avoid rendering techniques that overwrite the depth buffer with other data.
// If the above rules cannot be applied, the feature for automatic depth-based image stabilization should
// not be enabled.
//
// The frame of reference provided by the app can be stationary, or it can be attached to a device.
//


#ifndef LIBANGLE_RENDERER_D3D_D3D11_WINRT_HOLOGRAPHICNATIVEWINDOW_H_
#define LIBANGLE_RENDERER_D3D_D3D11_WINRT_HOLOGRAPHICNATIVEWINDOW_H_

#include "libANGLE/renderer/d3d/d3d11/winrt/HolographicSwapChain11.h"

#ifdef ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
#include "libANGLE/renderer/d3d/d3d11/winrt/InspectableNativeWindow.h"

#include <d3d11_4.h>
#include <memory>
#include <ppltasks.h>
#include <inspectable.h>
#include <windows.graphics.display.h>
#include <windows.graphics.directx.direct3d11.interop.h>

#include <mutex>

typedef __FITypedEventHandler_2_Windows__CGraphics__CHolographic__CHolographicSpace_Windows__CGraphics__CHolographic__CHolographicSpaceCameraAddedEventArgs IHolographicSpaceCameraAddedEventArgs;
typedef __FITypedEventHandler_2_Windows__CGraphics__CHolographic__CHolographicSpace_Windows__CGraphics__CHolographic__CHolographicSpaceCameraRemovedEventArgs IHolographicSpaceCameraRemovedEventArgs;

STDAPI CreateDirect3D11DeviceFromDXGIDevice(
    _In_         IDXGIDevice* dxgiDevice,
    _COM_Outptr_ IInspectable** graphicsDevice);

STDAPI CreateDirect3D11SurfaceFromDXGISurface(
    _In_         IDXGISurface* dgxiSurface,
    _COM_Outptr_ IInspectable** graphicsSurface);

namespace rx
{
long ConvertDipsToPixels(float dips);

class HolographicNativeWindow : public InspectableNativeWindow, public std::enable_shared_from_this<HolographicNativeWindow>
{
  public:
    ~HolographicNativeWindow();
    HolographicNativeWindow& operator=(HolographicNativeWindow const& rhs) =delete;

    bool initialize(EGLNativeWindowType holographicSpace, IPropertySet *propertySet) override;
    HRESULT createSwapChain(ID3D11Device *device,
                            DXGIFactory *factory,
                            DXGI_FORMAT format,
                            unsigned int width,
                            unsigned int height,
                            bool containsAlpha,
                            DXGISwapChain **swapChain) override;

    static bool IsInitialized() { return mInitialized; }
    
    void setRenderer11(Renderer11* renderer) { mRenderer = renderer; };
    HRESULT setD3DDevice(ID3D11Device *device);

    void addHolographicCamera(ABI::Windows::Graphics::Holographic::IHolographicCamera *holographicCamera);
    void removeHolographicCamera(ABI::Windows::Graphics::Holographic::IHolographicCamera *holographicCamera);

    egl::Error SetSpatialFrameOfReference(IInspectable* value);
    egl::Error SetWaitForVBlank(bool const& isEnabled);

    ID3D11Device *getPreferredD3DDevice()  { return mPreferredD3DDevice.Get();  };

    std::vector<unsigned int> GetCameraIds();
    HRESULT GetHolographicSwapChain(size_t const& cameraIndex, HolographicSwapChain11** holographicSwapChain);

    HRESULT UpdateHolographicResources();
    ABI::Windows::Foundation::Collections::IVectorView<ABI::Windows::Graphics::Holographic::HolographicCameraPose*>*
        GetHolographicCameraPoses() { return mHolographicCameraPoses.Get(); };
    HRESULT GetHolographicRenderingParameters(UINT32 id, ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters** outParameters);
    HRESULT GetHolographicCameraPose(UINT32 id, ABI::Windows::Graphics::Holographic::IHolographicCameraPose** outPose);
    ABI::Windows::Graphics::Holographic::IHolographicFrame* GetHolographicFrame() { return mHolographicFrame.Get(); };
    ABI::Windows::Perception::Spatial::ISpatialCoordinateSystem* GetCoordinateSystem() { return mCoordinateSystem.Get(); };
    
    bool HasHolographicCameraListChanged()
    { 
        bool holographicCameraListChangedReturnValue = false;
        UseHolographicCameraResources<void>(
            [&, this](std::map<UINT32, std::unique_ptr<HolographicSwapChain11>>& cameraResourceMap)
            {
                holographicCameraListChangedReturnValue = mHolographicCameraListChanged;
                mHolographicCameraListChanged = false;
            });
        return holographicCameraListChangedReturnValue;
    }
    bool IsUpdateNeeded() { return mUpdateNeeded; }
	void ResetFrame();

  protected:
    HRESULT initializeHolographicSpaceUsingDirect3DDevice(ID3D11Device *device);
    HRESULT createPreferredD3DDevice(ID3D11Device *givenDevice);
    HRESULT getPreferredDeviceFeatureSupport();
    HRESULT scaleSwapChain(const SIZE &windowSize, const RECT &clientRect) override { return S_OK; };

    bool registerForHolographicCameraEvents();
    void unregisterForHolographicCameraEvents();

    EventRegistrationToken mCameraAddedEventToken;
    EventRegistrationToken mCameraRemovedEventToken;

  private:

    // Synchronizes access to the list of holographic cameras, which is updated using callbacks.
    template<typename RetType, typename LCallback>
    RetType UseHolographicCameraResources(const LCallback& callback);

    // Back buffer resources, etc. for attached holographic cameras.
    std::map<UINT32, std::unique_ptr<HolographicSwapChain11>>       mHolographicCameras;
    std::mutex                                                      mHolographicCamerasLock;
    ComPtr<ABI::Windows::Foundation::Collections::IVectorView<ABI::Windows::Graphics::Holographic::HolographicCameraPose*>> mHolographicCameraPoses;
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicFrame>  mHolographicFrame;
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicSpace>  mHolographicSpace;
    

    ComPtr<ABI::Windows::Perception::Spatial::ISpatialCoordinateSystem> mCoordinateSystem;

    // SpatialLocator that is attached to the primary camera.
    ComPtr<ABI::Windows::Perception::Spatial::ISpatialLocator> mLocator;

    // A reference frame attached to the holographic camera.
    ComPtr<ABI::Windows::Perception::Spatial::ISpatialLocatorAttachedFrameOfReference> mAttachedReferenceFrame;

    // A reference frame placed in the environment.
    ComPtr<ABI::Windows::Perception::Spatial::ISpatialStationaryFrameOfReference> mStationaryReferenceFrame;

    bool                                 mSupportsVprt;
    ComPtr<ID3D11Device4>                mPreferredD3DDevice;
    ComPtr<ID3D11DeviceContext3>         mPreferredD3DContext;
    ComPtr<IDXGIDevice>                  mPreferredDxgiDevice;
    ComPtr<IDXGIAdapter3>                mPreferredDxgiAdapter;
    ComPtr<IMap<HSTRING, IInspectable*>> mPropertyMap;

    Renderer11*     mRenderer;

    unsigned int    mCameraCount                    = 0;
    bool            mHolographicCameraListChanged   = false;
    bool            mUpdateNeeded                   = true;

    static bool     mInitialized;
};

[uuid(51d90891-02cc-4b67-96af-c7dc8ddf9fec)]
class CameraAddedEventHandler :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IHolographicSpaceCameraAddedEventArgs>
{
  public:
    CameraAddedEventHandler() { }
    HRESULT RuntimeClassInitialize(std::shared_ptr<HolographicNativeWindow> host)
    {
        if (!host)
        {
            return E_INVALIDARG;
        }

        mHost = host;
        return S_OK;
    }

    // IHolographicSpaceCameraAddedEventArgs
    IFACEMETHOD(Invoke)(ABI::Windows::Graphics::Holographic::IHolographicSpace *sender, 
                        ABI::Windows::Graphics::Holographic::IHolographicSpaceCameraAddedEventArgs *cameraAddedEventArgs)
    {
        std::shared_ptr<HolographicNativeWindow> host = mHost.lock();
        if (host)
        {
            ComPtr<ABI::Windows::Foundation::IDeferral> deferral;
            HRESULT hr = cameraAddedEventArgs->GetDeferral(deferral.GetAddressOf());

            ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCamera> holographicCamera;
            if (SUCCEEDED(hr))
            {
                hr = cameraAddedEventArgs->get_Camera(holographicCamera.GetAddressOf());
                if (SUCCEEDED(hr))
                {
                    Concurrency::create_task([this, host, deferral, holographicCamera]()
                    {
                        host->addHolographicCamera(holographicCamera.Get());

                        // Holographic frame predictions will not include any information about this camera until
                        // the deferral is completed.
                        deferral->Complete();
                    });
                }
                else
                {
                    deferral->Complete();
                }
            }
        }

        return S_OK;
    }

  private:
    std::weak_ptr<HolographicNativeWindow> mHost;
};

[uuid(a6460896-0c10-4abd-9d43-4f444a90c831)]
class CameraRemovedEventHandler :
    public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>, IHolographicSpaceCameraRemovedEventArgs>
{
public:
    CameraRemovedEventHandler() { }
    HRESULT RuntimeClassInitialize(std::shared_ptr<HolographicNativeWindow> host)
    {
        if (!host)
        {
            return E_INVALIDARG;
        }

        mHost = host;
        return S_OK;
    }

    // IHolographicSpaceCameraRemovedEventArgs
    IFACEMETHOD(Invoke)(ABI::Windows::Graphics::Holographic::IHolographicSpace *sender,
                        ABI::Windows::Graphics::Holographic::IHolographicSpaceCameraRemovedEventArgs *cameraRemovedEventArgs)
    {
        std::shared_ptr<HolographicNativeWindow> host = mHost.lock();
        if (host)
        {
            ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCamera> holographicCamera;
            HRESULT hr = cameraRemovedEventArgs->get_Camera(holographicCamera.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                host->removeHolographicCamera(holographicCamera.Get());
            }
        }

        return S_OK;
    }

private:
    std::weak_ptr<HolographicNativeWindow> mHost;
};

// Device-based resources for holographic cameras are stored in a std::map. Access this list by providing a
// callback to this function, and the std::map will be guarded from add and remove
// events until the callback returns. The callback is processed immediately and must
// not contain any nested calls to UseHolographicCameraResources.
// The callback takes a parameter of type std::map<UINT32, std::unique_ptr<DX::CameraResources>>&
// through which the list of cameras will be accessed.
template<typename RetType, typename LCallback>
RetType HolographicNativeWindow::UseHolographicCameraResources(const LCallback& callback)
{
    std::lock_guard<std::mutex> guard(mHolographicCamerasLock);
    return callback(mHolographicCameras);
}



HRESULT GetCoreWindowSizeInPixels(const ComPtr<ABI::Windows::UI::Core::ICoreWindow>& coreWindow, SIZE *windowSize);
}

#else // ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
namespace rx
{
class HolographicNativeWindow
{
public:
    static bool IsInitialized() { return false; }
};
}
#endif // ANGLE_ENABLE_WINDOWS_HOLOGRAPHIC
#endif // LIBANGLE_RENDERER_D3D_D3D11_WINRT_HOLOGRAPHICNATIVEWINDOW_H_
