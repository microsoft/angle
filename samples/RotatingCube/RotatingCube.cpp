#include "pch.h"
#include "RotatingCube.h"
#include "BasicTimer.h"

#define STRINGIFY(x) #x

const char g_colorVertexShader[] = STRINGIFY(
precision mediump float;
attribute vec3 a_position;
attribute vec3 a_color;
varying vec4 v_color;
uniform mat4 u_mvp;
void main(void)
{
    gl_Position = u_mvp * vec4(a_position, 1);
    v_color = vec4(a_color, 1);
}
);

const char g_colorFragmentShader[] = STRINGIFY(
precision mediump float;
varying vec4 v_color;
void main(void)
{
    gl_FragColor = v_color;
}
);

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;

struct VertexPositionColor
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;
};

RotatingCube::RotatingCube() :
	m_windowClosed(false),
	m_windowVisible(true)
{
}

void RotatingCube::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &RotatingCube::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &RotatingCube::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &RotatingCube::OnResuming);
}

void RotatingCube::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &RotatingCube::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &RotatingCube::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &RotatingCube::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &RotatingCube::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &RotatingCube::OnPointerMoved);

	m_orientation = DisplayProperties::CurrentOrientation;
    m_windowBounds = window->Bounds;

    esInitContext ( &m_esContext );
    m_esContext.hWnd.window = CoreWindow::GetForCurrentThread();

    //title, width, and height are unused, but included for backwards compatibility
    esCreateWindow ( &m_esContext, nullptr, 0, 0, ES_WINDOW_RGB | ES_WINDOW_DEPTH );

    m_colorProgram = esLoadProgram(g_colorVertexShader, g_colorFragmentShader);
    a_positionColor = glGetAttribLocation(m_colorProgram, "a_position");
    a_colorColor = glGetAttribLocation(m_colorProgram, "a_color");
    u_mvpColor = glGetUniformLocation(m_colorProgram, "u_mvp");

    m_projectionMatrix = XMMatrixPerspectiveFovRH(70.0f * XM_PI / 180.0f, m_windowBounds.Width / m_windowBounds.Height, 0.01f, 100.0f);

    glEnable(GL_DEPTH_TEST);
}

void RotatingCube::Load(Platform::String^ entryPoint)
{
}

void RotatingCube::Run()
{
	BasicTimer^ timer = ref new BasicTimer();

	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            
            XMVECTOR eye = XMVectorSet(0.0f, 0.7f, 1.5f, 0.0f);
	        XMVECTOR at = XMVectorSet(0.0f, -0.1f, 0.0f, 0.0f);
	        XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
            
	        XMMATRIX viewMatrix = XMMatrixLookAtRH(eye, at, up);
	        XMMATRIX modelMatrix = XMMatrixRotationY(timer->Total * XM_PIDIV4);

            XMFLOAT4X4 mvp;
            XMStoreFloat4x4(&mvp, (XMMatrixMultiply(XMMatrixMultiply(modelMatrix, viewMatrix), m_projectionMatrix)));

            glClearColor(0.098f, 0.098f, 0.439f, 1.000f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(m_colorProgram);
            glUniformMatrix4fv(u_mvpColor, 1, GL_FALSE, &mvp.m[0][0]);

            VertexPositionColor cubeVertices[] = 
		    {
			    {XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0.0f, 0.0f, 0.0f)},
			    {XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0.0f, 0.0f, 1.0f)},
			    {XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0.0f, 1.0f, 0.0f)},
			    {XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0.0f, 1.0f, 1.0f)},
			    {XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1.0f, 0.0f, 0.0f)},
			    {XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1.0f, 0.0f, 1.0f)},
			    {XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1.0f, 1.0f, 0.0f)},
			    {XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f)},
		    };
            unsigned short cubeIndices[] = 
            {

                0,2,1, // -x
                1,2,3,

                4,5,6, // +x
                5,7,6,

                0,1,5, // -y
                0,5,4,

                2,6,7, // +y
                2,7,3,

                0,4,6, // -z
                0,6,2,

                1,3,7, // +z
                1,7,5,
            };

            glEnableVertexAttribArray(a_positionColor);
            glEnableVertexAttribArray(a_colorColor);
            glVertexAttribPointer(a_positionColor, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionColor), cubeVertices);
            glVertexAttribPointer(a_colorColor, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPositionColor), reinterpret_cast<char*>(cubeVertices) + sizeof(XMFLOAT3));
            glDrawElements(GL_TRIANGLES, ARRAYSIZE(cubeIndices), GL_UNSIGNED_SHORT, cubeIndices);
            glDisableVertexAttribArray(a_positionColor);
            glDisableVertexAttribArray(a_colorColor);

            eglSwapBuffers(m_esContext.eglDisplay, m_esContext.eglSurface);
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void RotatingCube::Uninitialize()
{
}

static float ConvertDipsToPixels(float dips)
{
    static const float dipsPerInch = 96.0f;
    return floor(dips * DisplayProperties::LogicalDpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

void RotatingCube::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	//m_renderer->UpdateForWindowSizeChange();
        //m_renderer->UpdateForWindowSizeChange();
    if (sender->Bounds.Width  != m_windowBounds.Width ||
        sender->Bounds.Height != m_windowBounds.Height ||
        m_orientation != DisplayProperties::CurrentOrientation)
    {
        // Internally calls DX11's version of flush
        glFlush();

        // Store the window bounds so the next time we get a SizeChanged event we can
        // avoid rebuilding everything if the size is identical.
        m_windowBounds = sender->Bounds;

        // Calculate the necessary swap chain and render target size in pixels.
        float windowWidth = ConvertDipsToPixels(m_windowBounds.Width);
        float windowHeight = ConvertDipsToPixels(m_windowBounds.Height);

        // The width and height of the swap chain must be based on the window's
        // landscape-oriented width and height. If the window is in a portrait
        // orientation, the dimensions must be reversed.
        m_orientation = DisplayProperties::CurrentOrientation;
        //bool swapDimensions =
        //    m_orientation == DisplayOrientations::Portrait ||
        //    m_orientation == DisplayOrientations::PortraitFlipped;
        //if(swapDimensions)
        //    std::swap(windowWidth, windowHeight);

        // Actually resize the underlying swapchain
        esResizeWindow(&m_esContext, static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight));
        glViewport(0, 0, static_cast<UINT>(windowWidth), static_cast<UINT>(windowHeight));
        m_projectionMatrix = XMMatrixPerspectiveFovRH(70.0f * XM_PI / 180.0f, windowWidth / windowHeight, 0.01f, 100.0f);
    }
}

void RotatingCube::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void RotatingCube::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void RotatingCube::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void RotatingCube::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void RotatingCube::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void RotatingCube::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void RotatingCube::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new RotatingCube();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
