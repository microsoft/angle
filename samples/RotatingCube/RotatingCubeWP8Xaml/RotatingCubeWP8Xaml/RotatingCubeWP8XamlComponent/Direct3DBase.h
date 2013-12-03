#pragma once

#include "DirectXHelper.h"
#include "esUtil.h"
#include "winrtangle.h"

// Helper class that initializes DirectX APIs for 3D rendering.
ref class Direct3DBase abstract
{
internal:
	Direct3DBase();

	virtual void Initialize(_In_ ID3D11Device1* device);
	virtual void CreateDeviceResources();
	virtual void UpdateDevice(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView);
	virtual void CreateWindowSizeDependentResources();
	virtual void UpdateForWindowSizeChange(float width, float height);
	virtual void Render() = 0;
	virtual void OnOrientationChanged() = 0;

protected private:
	// Direct3D Objects.
	Microsoft::WRL::ComPtr<ID3D11Device1> m_d3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1> m_d3dContext;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_renderTargetView;

    ESContext m_esContext;
	Microsoft::WRL::ComPtr<IWinPhone8XamlD3DWindow> m_eglPhoneWindow;
	Microsoft::WRL::ComPtr<IWinrtEglWindow> m_eglWindow;

	// Cached renderer properties.
	Windows::Foundation::Size m_renderTargetSize;
	Windows::Foundation::Rect m_windowBounds;

	bool m_bInitialized;
};