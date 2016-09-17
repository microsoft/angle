//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HolographicNativeWindow.cpp: NativeWindow for managing windows based on Windows Holographic app views.
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


#include "libANGLE/renderer/d3d/d3d11/winrt/HolographicNativeWindow.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"

using namespace ABI::Windows::Foundation::Collections;

namespace rx
{

bool HolographicNativeWindow::mInitialized = false;

HolographicNativeWindow::~HolographicNativeWindow()
{
    unregisterForHolographicCameraEvents();
}

bool HolographicNativeWindow::initialize(EGLNativeWindowType holographicSpace, IPropertySet *propertySet)
{
    mInitialized = true;

    ComPtr<IPropertySet> spProps = propertySet;
    ComPtr<IInspectable> spSpace = holographicSpace;
    SIZE swapChainSize = {};
    HRESULT result = S_OK;

    if (SUCCEEDED(result))
    {
        result = spSpace.As(&mHolographicSpace);
    }

    ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable*>> spPropertyMap;
    if (SUCCEEDED(result))
    {
        result = spProps.As(&spPropertyMap);
    }

    {
        // Look for the presence of the EGLNativeWindowType in the property set.
        boolean hasEglBaseCoordinateSystemProperty = false;
        if (SUCCEEDED(result))
        {
            result = spPropertyMap->HasKey(HStringReference(EGLBaseCoordinateSystemProperty).Get(), &hasEglBaseCoordinateSystemProperty);
        }

        // If the IPropertySet does not contain the required EGLBaseCoordinateSystemProperty key, the property set is
        // considered invalid.
        if (SUCCEEDED(result) && !hasEglBaseCoordinateSystemProperty)
        {
            ERR("Could not find EGLBaseCoordinateSystemProperty in IPropertySet. Valid EGLBaseCoordinateSystemProperty values include ISpatialStationaryFrameOfReference and IDeviceAttachedFrameOfReference");
            return false;
        }

        // The EGLBaseCoordinateSystemProperty property exists, so retrieve the IInspectable that represents the EGLBaseCoordinateSystemProperty.
        ComPtr<IInspectable> frameOfReference;
        if (SUCCEEDED(result) && hasEglBaseCoordinateSystemProperty)
        {
            result = spPropertyMap->Lookup(HStringReference(EGLBaseCoordinateSystemProperty).Get(), &frameOfReference);
        }

        if (SetSpatialFrameOfReference(frameOfReference.Get()).getCode() == EGL_BAD_PARAMETER)
        {
            result = E_INVALIDARG;
        }
    }

    {
        HRESULT result = S_OK;

        // Look for the presence of the EGLAutomaticStereoRenderingProperty property in the property set.
        boolean hasEglAutomaticStereoRenderingProperty = false;
        if (SUCCEEDED(result))
        {
            result = spPropertyMap->HasKey(HStringReference(EGLAutomaticStereoRenderingProperty).Get(), &hasEglAutomaticStereoRenderingProperty);
        }

        // The property exists, so retrieve the IInspectable that represents it.
        ComPtr<IInspectable> enableAutomaticStereoRenderingPropertyInspectable;
        if (SUCCEEDED(result) && hasEglAutomaticStereoRenderingProperty)
        {
            result = spPropertyMap->Lookup(HStringReference(EGLAutomaticStereoRenderingProperty).Get(), &enableAutomaticStereoRenderingPropertyInspectable);
        }

        ComPtr<IPropertyValue> enableAutomaticStereoRenderingProperty;
        if (SUCCEEDED(result) && (enableAutomaticStereoRenderingPropertyInspectable != nullptr))
        {
            result = enableAutomaticStereoRenderingPropertyInspectable.As(&enableAutomaticStereoRenderingProperty);
        }

        boolean enableAutomaticStereoRendering = false;
        if (SUCCEEDED(result) && (enableAutomaticStereoRenderingProperty != nullptr))
        {
            result = enableAutomaticStereoRenderingProperty->GetBoolean(&enableAutomaticStereoRendering);
        }

        HolographicSwapChain11::setIsAutomaticStereoRenderingEnabled(enableAutomaticStereoRendering);
    }

    {
        HRESULT result = S_OK;

        // Look for the presence of the EGLAutomaticDepthBasedImageStabilizationProperty property in the property set.
        boolean hasEglAutomaticDepthBasedImageStabilizationProperty = false;
        if (SUCCEEDED(result))
        {
            result = spPropertyMap->HasKey(HStringReference(EGLAutomaticDepthBasedImageStabilizationProperty).Get(), &hasEglAutomaticDepthBasedImageStabilizationProperty);
        }

        // The property exists, so retrieve the IInspectable that represents it.
        ComPtr<IInspectable> enableAutomaticDepthBasedImageStabilizationPropertyInspectable;
        if (SUCCEEDED(result) && hasEglAutomaticDepthBasedImageStabilizationProperty)
        {
            result = spPropertyMap->Lookup(HStringReference(EGLAutomaticStereoRenderingProperty).Get(), &enableAutomaticDepthBasedImageStabilizationPropertyInspectable);
        }

        ComPtr<IPropertyValue> enableAutomaticDepthBasedImageStabilizationProperty;
        if (SUCCEEDED(result) && (enableAutomaticDepthBasedImageStabilizationPropertyInspectable != nullptr))
        {
            result = enableAutomaticDepthBasedImageStabilizationPropertyInspectable.As(&enableAutomaticDepthBasedImageStabilizationProperty);
        }

        boolean enableAutomaticDepthBasedImageStabilization = false;
        if (SUCCEEDED(result) && (enableAutomaticDepthBasedImageStabilizationProperty != nullptr))
        {
            result = enableAutomaticDepthBasedImageStabilizationProperty->GetBoolean(&enableAutomaticDepthBasedImageStabilization);
        }

        HolographicSwapChain11::setIsAutomaticDepthBasedImageStabilizationEnabled(enableAutomaticDepthBasedImageStabilization);
    }

    // Subscribe to holographic cameras.
    if (SUCCEEDED(result))
    {
        return registerForHolographicCameraEvents();
    }

    return false;
}

bool HolographicNativeWindow::registerForHolographicCameraEvents()
{
    ComPtr<IHolographicSpaceCameraAddedEventArgs> spCameraAddedHandler;
    HRESULT result = Microsoft::WRL::MakeAndInitialize<CameraAddedEventHandler>(spCameraAddedHandler.ReleaseAndGetAddressOf(), this->shared_from_this());
    if (SUCCEEDED(result))
    {
        result = mHolographicSpace->add_CameraAdded(spCameraAddedHandler.Get(), &mCameraAddedEventToken);
    }

    ComPtr<IHolographicSpaceCameraRemovedEventArgs> spCameraRemovedHandler;
    result = Microsoft::WRL::MakeAndInitialize<CameraRemovedEventHandler>(spCameraRemovedHandler.ReleaseAndGetAddressOf(), this->shared_from_this());
    if (SUCCEEDED(result))
    {
        result = mHolographicSpace->add_CameraRemoved(spCameraRemovedHandler.Get(), &mCameraRemovedEventToken);
    }

    if (SUCCEEDED(result))
    {
        return true;
    }

    return false;
}

void HolographicNativeWindow::unregisterForHolographicCameraEvents()
{
    if (mHolographicSpace)
    {
        (void) mHolographicSpace->remove_CameraAdded(mCameraAddedEventToken);
        (void) mHolographicSpace->remove_CameraRemoved(mCameraRemovedEventToken);
    }
    mCameraAddedEventToken.value = 0;
    mCameraRemovedEventToken.value = 0;
}

#if defined(_DEBUG)
// Check for SDK Layer support.
inline bool SdkLayersAvailable()
{
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
        0,
        D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
        nullptr,                    // Any feature level will do.
        0,
        D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
        nullptr,                    // No need to keep the D3D device reference.
        nullptr,                    // No need to know the feature level.
        nullptr                     // No need to keep the D3D device context reference.
    );

    return SUCCEEDED(hr);
}
#endif

HRESULT HolographicNativeWindow::createPreferredD3DDevice(ID3D11Device *givenDevice)
{
    // This flag adds support for surfaces with a different color channel ordering
    // than the API default. It is required for compatibility with Direct2D.
    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
    if (SdkLayersAvailable())
    {
        // If the project is in a debug build, enable debugging via SDK Layers with this flag.
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    }
#endif

    // This array defines the set of DirectX hardware feature levels this app will support.
    // Note the ordering should be preserved.
    // Note that HoloLens supports feature level 11.1. The HoloLens emulator is also capable
    // of running on graphics cards starting with feature level 10.0.
    D3D_FEATURE_LEVEL featureLevels [] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    // Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> spNewDevice;
    ComPtr<ID3D11DeviceContext> spNewContext;
    HRESULT hr = D3D11CreateDevice(
        mPreferredDxgiAdapter.Get(),// The primary DXGI adapter determined to support a Windows Holographic device.
        D3D_DRIVER_TYPE_HARDWARE,   // Create a device using the hardware graphics driver.
        0,                          // Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
        creationFlags,              // Set debug and Direct2D compatibility flags.
        featureLevels,              // List of feature levels this app can support.
        ARRAYSIZE(featureLevels),   // Size of the list above.
        D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Windows Store apps.
        &spNewDevice,               // Returns the Direct3D device created.
        nullptr,
        &spNewContext               // Returns the device immediate context.
    );

    // Store pointers to the Direct3D device and immediate context.
    if (SUCCEEDED(hr))
    {
        hr = spNewDevice.As(&mPreferredD3DDevice);
    }

    if (SUCCEEDED(hr))
    {
        hr = spNewContext.As(&mPreferredD3DContext);
    }

    if (FAILED(hr))
    {
        // If the initialization fails, fall back to whatever D3D device we were given.
        ComPtr<ID3D11Device> spDevice = givenDevice;
        hr = spDevice.As(&mPreferredD3DDevice);

        if (SUCCEEDED(hr))
        {
            mPreferredD3DDevice->GetImmediateContext3(mPreferredD3DContext.ReleaseAndGetAddressOf());
        }

        mPreferredDxgiDevice.Reset();
        mPreferredDxgiAdapter.Reset();
    }

    // Acquire the DXGI interface for the Direct3D device.
    if (SUCCEEDED(hr))
    {
        hr = mPreferredD3DDevice.As(&mPreferredDxgiDevice);
    }

    return hr;
}

// Returns the current list of HolographicCamera IDs.
std::vector<unsigned int> HolographicNativeWindow::GetCameraIds()
{
    return UseHolographicCameraResources<std::vector<unsigned int>>(
        [&](std::map<UINT32, std::unique_ptr<HolographicSwapChain11>>& holographicCameras)
    {
        std::vector<unsigned int> cameraIds;
        for each (auto const& camera in holographicCameras)
        {
            if (camera.second != nullptr)
            {
                cameraIds.push_back(camera.first);
            }
        }
        return cameraIds;
    });
}

// Returns a holographic swap chain, by HolographicCamera ID.
HRESULT HolographicNativeWindow::GetHolographicSwapChain(size_t const& cameraIndex, HolographicSwapChain11** holographicSwapChain)
{
    if (holographicSwapChain == nullptr)
    {
        return E_POINTER;
    }

    if (mHolographicCameras[cameraIndex] != nullptr)
    {
        return UseHolographicCameraResources<HRESULT>(
            [&](std::map<UINT32, std::unique_ptr<HolographicSwapChain11>>& holographicCameras)
        {
            *holographicSwapChain = holographicCameras[cameraIndex].get();
            return S_OK;
        });
    }
    else
    {
        return E_BOUNDS;
    }
}

HRESULT HolographicNativeWindow::initializeHolographicSpaceUsingDirect3DDevice(ID3D11Device *device)
{
    HRESULT hr = S_OK;
    if (device == nullptr)
    {
        return E_INVALIDARG;
    }
    else
    {
        hr = device->QueryInterface(IID_PPV_ARGS(mPreferredD3DDevice.GetAddressOf()));
    }

    // The holographic space might need to determine which adapter supports
    // holograms, in which case it will specify a non-zero PrimaryAdapterId.

    ABI::Windows::Graphics::Holographic::HolographicAdapterId primaryAdapterId = { 0, 0 };
    if (SUCCEEDED(hr))
    {
        hr = mHolographicSpace->get_PrimaryAdapterId(&primaryAdapterId);
    }
    LUID id =
    {
        primaryAdapterId.LowPart,
        primaryAdapterId.HighPart
    };

    ComPtr<IDXGIDevice> preferredDxgiDevice;

    // When a primary adapter ID is given to the app, the app should find
    // the corresponding DXGI adapter and use it to create Direct3D devices
    // and device contexts. Otherwise, there is no restriction on the DXGI
    // adapter the app can use.
    if ((id.HighPart != 0) && (id.LowPart != 0))
    {
        UINT createFlags = 0;
#if defined(_DEBUG)
        if (SdkLayersAvailable())
        {
            createFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
#endif
        // Get the DXGI adapter.

        // Create the DXGI factory.
        ComPtr<IDXGIFactory1> dxgiFactory;
        hr = CreateDXGIFactory2(createFlags, IID_PPV_ARGS(&dxgiFactory));

        ComPtr<IDXGIFactory4> dxgiFactory4;
        if (SUCCEEDED(hr))
        {
            hr = dxgiFactory.As(&dxgiFactory4);
        }

        // Retrieve the adapter specified by the holographic space.
        if (SUCCEEDED(hr))
        {
            dxgiFactory4->EnumAdapterByLuid(id, IID_PPV_ARGS(&mPreferredDxgiAdapter));
        }

        // Check to see if the D3D device is using the preferred adapter.
        if (SUCCEEDED(hr))
        {
            // Acquire the DXGI interface for the Direct3D device.
            ComPtr<ID3D11Device> spDevice = device;
            ComPtr<IDXGIDevice3> currentDxgiDevice;
            hr = spDevice.As(&currentDxgiDevice);

            ComPtr<IDXGIAdapter> currentDxgiAdapter;
            if (SUCCEEDED(hr))
            {
                hr = currentDxgiDevice->GetAdapter(&currentDxgiAdapter);
            }

            if (SUCCEEDED(hr))
            {
                if (currentDxgiAdapter != mPreferredDxgiAdapter)
                {
                    createPreferredD3DDevice(device);
                }
                else
                {
                    hr = spDevice.As(&mPreferredD3DDevice);
                    mPreferredDxgiDevice = currentDxgiDevice;
                }
            }
        }

    }
    else if (SUCCEEDED(hr))
    {
        hr = mPreferredD3DDevice.As(&mPreferredDxgiDevice);

        // clear out previous entries
        mPreferredD3DContext.Reset();
        mPreferredDxgiAdapter.Reset();
    }

    // Wrap the native device using a WinRT interop object.
    ComPtr<IInspectable> inspectableDevice;
    if (SUCCEEDED(hr))
    {
        hr = CreateDirect3D11DeviceFromDXGIDevice(mPreferredDxgiDevice.Get(), &inspectableDevice);
    }
    ComPtr<ABI::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice> interopDevice;
    if (SUCCEEDED(hr))
    {
        hr = inspectableDevice.As(&interopDevice);
    }

    // Provide the Direct3D device to the holographic space.
    if (SUCCEEDED(hr))
    {
        hr = mHolographicSpace->SetDirect3D11Device(interopDevice.Get());
    }

    return hr;
}

HRESULT HolographicNativeWindow::getPreferredDeviceFeatureSupport()
{
    // Check for device support for the optional feature that allows setting the render target array index from the vertex shader stage.
    D3D11_FEATURE_DATA_D3D11_OPTIONS3 options;
    HRESULT hr = mPreferredD3DDevice->CheckFeatureSupport(D3D11_FEATURE_D3D11_OPTIONS3, &options, sizeof(options));
    
    if (SUCCEEDED(hr))
    {
        if (options.VPAndRTArrayIndexFromAnyShaderFeedingRasterizer)
        {
            mSupportsVprt = true;
        }
        else
        {
            mSupportsVprt = false;
        }
    }

    return hr;
}

// Initializes the HolographicSpace.
HRESULT HolographicNativeWindow::setD3DDevice(ID3D11Device *device)
{
    if (device == NULL)
    {
        return E_INVALIDARG;
    }

    if (mHolographicSpace == NULL)
    {
        return E_UNEXPECTED;
    }

    HRESULT result = initializeHolographicSpaceUsingDirect3DDevice(device);

    if (SUCCEEDED(result))
    {
        result = getPreferredDeviceFeatureSupport();
    }

    return result;
}

HRESULT HolographicNativeWindow::createSwapChain(ID3D11Device *device,
                                                DXGIFactory *    /*factory*/,
                                                DXGI_FORMAT      /*format*/,
                                                unsigned int     /*width*/,
                                                unsigned int     /*height*/,
                                                bool             /*containsAlpha*/,
                                                DXGISwapChain ** /*swapChain*/)
{
    return setD3DDevice(device);
}

void HolographicNativeWindow::addHolographicCamera(ABI::Windows::Graphics::Holographic::IHolographicCamera *holographicCamera)
{
    UseHolographicCameraResources<void>([this, holographicCamera](std::map<UINT32, std::unique_ptr<HolographicSwapChain11>>& cameraResourceMap)
    {
        // Add the holographic camera to the list.
        UINT32 id;
        (void) holographicCamera->get_Id(&id);
        HANDLE shareHandle = (void*) 0;
        cameraResourceMap[id] = std::make_unique<HolographicSwapChain11>(mRenderer, this, shareHandle, holographicCamera);

        // Indicate to the HolographicNativeWindow that the camera list has changed.
        mHolographicCameraListChanged = true;
    });
}

void HolographicNativeWindow::removeHolographicCamera(ABI::Windows::Graphics::Holographic::IHolographicCamera *holographicCamera)
{
    // Ensure system references to the back buffer are released by clearing the render
    // target from the graphics pipeline state, and then flushing the Direct3D context.
    auto context = mRenderer->getDeviceContext();
    ID3D11RenderTargetView* nullViews[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT] = { nullptr };
    context->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
    context->VSSetShaderResources(0, 0, nullptr);
    context->GSSetShaderResources(0, 0, nullptr);
    context->DSSetShaderResources(0, 0, nullptr);
    context->HSSetShaderResources(0, 0, nullptr);
    context->PSSetShaderResources(0, 0, nullptr);
    context->CSSetShaderResources(0, 0, nullptr);
    context->Flush();

    UseHolographicCameraResources<void>([this, holographicCamera](std::map<UINT32, std::unique_ptr<HolographicSwapChain11>>& cameraResourceMap)
    {
        // Indicate to the HolographicNativeWindow that the camera list has changed.
        mHolographicCameraListChanged = true;

        // Erase the holographic camera from the list.
        UINT32 id;
        (void) holographicCamera->get_Id(&id);
        cameraResourceMap.erase(id);
    });
}

egl::Error HolographicNativeWindow::SetSpatialFrameOfReference(IInspectable * value)
{
    HRESULT result = S_OK;

    if (value == nullptr)
    {
        result = E_INVALIDARG;
    }

    ComPtr<IInspectable> frameOfReference(value);
    HRESULT isStationaryFrameOfReference;
    if (SUCCEEDED(result))
    {
        isStationaryFrameOfReference = frameOfReference.As(&mStationaryReferenceFrame);
    }
    if (FAILED(isStationaryFrameOfReference))
    {
        mStationaryReferenceFrame.Reset();
        result = frameOfReference.As(&mAttachedReferenceFrame);
    }

    if (SUCCEEDED(result))
    {
        return egl::Error(EGL_SUCCESS);
    }
    else
    {
        return egl::Error(EGL_BAD_PARAMETER);
    }
}

egl::Error HolographicNativeWindow::SetWaitForVBlank(bool const& isEnabled)
{
    HolographicSwapChain11::setIsWaitForVBlankEnabled(isEnabled);

    return egl::Error(EGL_SUCCESS);
}

HRESULT HolographicNativeWindow::UpdateHolographicResources()
{
    if (mHolographicSpace == nullptr)
    {
        if (mRenderer == nullptr)
        {
            // Holographic window is not being used.
            return S_OK;
        }
        else
        {
            return E_UNEXPECTED;
        }
    }
    
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicFrame> holographicFrame;
    HRESULT hr = mHolographicSpace->CreateNextFrame(holographicFrame.GetAddressOf());
    
    if (SUCCEEDED(hr))
    {
        mHolographicFrame = holographicFrame;
    }
    else
    {
        if (mHolographicFrame != nullptr)
        {
            ABI::Windows::Graphics::Holographic::HolographicFramePresentResult result;
            mHolographicFrame->PresentUsingCurrentPredictionWithBehavior(
                ABI::Windows::Graphics::Holographic::HolographicFramePresentWaitBehavior_DoNotWaitForFrameToFinish,
                &result);
        }

        // An error condition was encountered. We might not be able to present.
        // Reset the per-frame update state, so that subsequent frames will try again.
        ResetFrame();
        return hr;
    }

    // Get a prediction of where holographic cameras will be when this frame
    // is presented.
    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicFramePrediction> prediction;
    if (SUCCEEDED(hr))
    {
        hr = mHolographicFrame->get_CurrentPrediction(prediction.GetAddressOf());
    }

    // Get the updated set of camera poses.
    if (SUCCEEDED(hr))
    {
        hr = prediction->get_CameraPoses(mHolographicCameraPoses.ReleaseAndGetAddressOf());
    }

    // Get the coordinate system.
    if (SUCCEEDED(hr))
    {
        if (mStationaryReferenceFrame != nullptr)
        {
            hr = mStationaryReferenceFrame->get_CoordinateSystem(mCoordinateSystem.GetAddressOf());
        }
        else if (mAttachedReferenceFrame != nullptr)
        {
            ComPtr<ABI::Windows::Perception::IPerceptionTimestamp> timestamp;
            hr = prediction->get_Timestamp(timestamp.GetAddressOf());
            if (SUCCEEDED(hr))
            {
                hr = mAttachedReferenceFrame->GetStationaryCoordinateSystemAtTimestamp(timestamp.Get(), mCoordinateSystem.GetAddressOf());
            }
        }
    }

    mUpdateNeeded = false;

    return hr;
}

HRESULT HolographicNativeWindow::GetHolographicRenderingParameters(UINT32 id, ABI::Windows::Graphics::Holographic::IHolographicCameraRenderingParameters** outParameters)
{
    if (outParameters == nullptr)
    {
        return E_POINTER;
    }

    if (mHolographicCameraPoses == nullptr)
    {
        return E_FAIL;
    }

    unsigned int size = 0;
    HRESULT hr = mHolographicCameraPoses->get_Size(&size);
    if (FAILED(hr))
    {
        return E_UNEXPECTED;
    }
    if ((id+1) > size)
    {
        return E_BOUNDS;
    }

    ComPtr<ABI::Windows::Graphics::Holographic::IHolographicCameraPose> cameraPose;
    hr = mHolographicCameraPoses->GetAt(id, cameraPose.GetAddressOf());

    if (SUCCEEDED(hr))
    {
        hr = mHolographicFrame->GetRenderingParameters(cameraPose.Get(), outParameters);
    }

    return hr;
};

HRESULT HolographicNativeWindow::GetHolographicCameraPose(UINT32 id, ABI::Windows::Graphics::Holographic::IHolographicCameraPose** outPose)
{
    if (outPose == nullptr)
    {
        return E_POINTER;
    }

    if (mHolographicCameraPoses == nullptr)
    {
        return E_FAIL;
    }

    unsigned int size = 0;
    HRESULT hr = mHolographicCameraPoses->get_Size(&size);
    if (FAILED(hr))
    {
        return E_UNEXPECTED;
    }
    if ((id+1) > size)
    {
        return E_BOUNDS;
    }

    return mHolographicCameraPoses->GetAt(id, outPose);
};

void HolographicNativeWindow::ResetFrame()
{
    // Reset the frame update state, triggering a holographic resource update for
    // the next frame.
    mUpdateNeeded = true;
    mHolographicFrame.Reset();
    mCoordinateSystem.Reset();
    mHolographicCameraPoses.Reset();
}

}
