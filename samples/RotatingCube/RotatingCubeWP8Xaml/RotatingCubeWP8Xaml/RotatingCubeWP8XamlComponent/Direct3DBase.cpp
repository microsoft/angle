#include "pch.h"
#include "Direct3DBase.h"

using namespace DirectX;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;

// Constructor.
Direct3DBase::Direct3DBase()
	: m_bInitialized(false)
{
	m_windowBounds.Width = 0.0f;
	m_windowBounds.Height = 0.0f;
}

// Initialize the Direct3D resources required to run.
void Direct3DBase::Initialize(_In_ ID3D11Device1* device)
{
	// defer initialization of Angle until first call to UpdateDevice()
}

// These are the resources that depend on the device.
void Direct3DBase::CreateDeviceResources()
{
}

void Direct3DBase::UpdateDevice(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView)
{
	m_d3dContext = context;
	m_renderTargetView = renderTargetView;

	if(!m_bInitialized)
	{
		m_d3dDevice = device;

		esInitContext(&m_esContext);

		// we need to select the correct DirectX feature level depending on the platform
		// default is for WP8
		ANGLE_D3D_FEATURE_LEVEL featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_9_3;


		switch(device->GetFeatureLevel())
		{
		case D3D_FEATURE_LEVEL_11_0:
			featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_11_0;
			break;
		case ANGLE_D3D_FEATURE_LEVEL_10_1:
			featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_10_1;
			break;

		case ANGLE_D3D_FEATURE_LEVEL_10_0:
			featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_10_0;
			break;

		case ANGLE_D3D_FEATURE_LEVEL_9_3:
			featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_9_3;
			break;
				
		case ANGLE_D3D_FEATURE_LEVEL_9_2:
			featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_9_2;
			break;
					
		case ANGLE_D3D_FEATURE_LEVEL_9_1:
			featureLevel = ANGLE_D3D_FEATURE_LEVEL::ANGLE_D3D_FEATURE_LEVEL_9_1;
			break;
		}		


		HRESULT result = CreateWinPhone8XamlWindow(&m_eglPhoneWindow);
		if (SUCCEEDED(result))
		{
			m_eglPhoneWindow->Update(WINRT_EGL_IUNKNOWN(m_d3dDevice.Get()), WINRT_EGL_IUNKNOWN(m_d3dContext.Get()), WINRT_EGL_IUNKNOWN(m_renderTargetView.Get()));

			result = CreateWinrtEglWindow(WINRT_EGL_IUNKNOWN(m_eglPhoneWindow.Get()), featureLevel, m_eglWindow.GetAddressOf());
			if (SUCCEEDED(result))
			{
				m_esContext.hWnd = m_eglWindow;

				//title, width, and height are unused, but included for backwards compatibility
				esCreateWindow(&m_esContext, nullptr, 0, 0, ES_WINDOW_RGB | ES_WINDOW_DEPTH);

				// CreateResources();
			}
		}

		m_bInitialized = true;
	}



	if (m_d3dDevice.Get() != device)
	{
		m_d3dDevice = device;

		CreateDeviceResources();

		// Force call to CreateWindowSizeDependentResources.
		m_renderTargetSize.Width  = -1;
		m_renderTargetSize.Height = -1;
	}

	ComPtr<ID3D11Resource> renderTargetViewResource;
	m_renderTargetView->GetResource(&renderTargetViewResource);

	ComPtr<ID3D11Texture2D> backBuffer;
	DX::ThrowIfFailed(
		renderTargetViewResource.As(&backBuffer)
		);

	//auto m_orientation = Windows::Graphics::Display::DisplayProperties::CurrentOrientation;


	// Cache the rendertarget dimensions in our helper class for convenient use.
    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);

    if (m_renderTargetSize.Width  != static_cast<float>(backBufferDesc.Width) ||
        m_renderTargetSize.Height != static_cast<float>(backBufferDesc.Height))
    {
        m_renderTargetSize.Width  = static_cast<float>(backBufferDesc.Width);
        m_renderTargetSize.Height = static_cast<float>(backBufferDesc.Height);
        CreateDeviceResources();
    }


	m_eglPhoneWindow->Update(WINRT_EGL_IUNKNOWN(m_d3dDevice.Get()), WINRT_EGL_IUNKNOWN(m_d3dContext.Get()), WINRT_EGL_IUNKNOWN(m_renderTargetView.Get()));
}

// Allocate all memory resources that depend on the window size.
void Direct3DBase::CreateWindowSizeDependentResources()
{
#if 0
	// Create a depth stencil view.
	CD3D11_TEXTURE2D_DESC depthStencilDesc(
		DXGI_FORMAT_D24_UNORM_S8_UINT,
		static_cast<UINT>(m_renderTargetSize.Width),
		static_cast<UINT>(m_renderTargetSize.Height),
		1,
		1,
		D3D11_BIND_DEPTH_STENCIL
		);

	ComPtr<ID3D11Texture2D> depthStencil;
	DX::ThrowIfFailed(
		m_d3dDevice->CreateTexture2D(
		&depthStencilDesc,
		nullptr,
		&depthStencil
		)
		);

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
	DX::ThrowIfFailed(
		m_d3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		&m_depthStencilView
		)
		);  
#endif // 0
}

void Direct3DBase::UpdateForWindowSizeChange(float width, float height)
{
	if (m_windowBounds.Width  != width || m_windowBounds.Height != height)
	{
		m_windowBounds.Width  = width;
		m_windowBounds.Height = height;
		OnOrientationChanged();
	}
}