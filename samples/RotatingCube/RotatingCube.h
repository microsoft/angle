#pragma once

#include "pch.h"
#include "esUtil.h"
#include <GLES2/gl2ext.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <ppl.h>
#include <ppltasks.h>

using namespace DirectX;

ref class RotatingCube sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	RotatingCube();
	
	// IFrameworkView Methods.
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();

protected:
	// Event Handlers.
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnLogicalDpiChanged(Platform::Object^ sender);
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender, Platform::Object^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);

private:
    ESContext m_esContext;
	Windows::Foundation::Rect m_windowBounds;
	Windows::Graphics::Display::DisplayOrientations m_orientation;
    GLuint m_colorProgram;
    GLint a_positionColor;
    GLint a_colorColor;
    GLint u_mvpColor;
    XMMATRIX m_projectionMatrix;
	bool m_windowClosed;
	bool m_windowVisible;
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
