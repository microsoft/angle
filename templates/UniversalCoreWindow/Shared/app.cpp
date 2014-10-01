//
// This file demonstrates how to initialize EGL in a Windows Store app, using ICoreWindow.
//

#include "pch.h"
#include "app.h"
#include "HelloTriangleRenderer.h"

using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Graphics::Display;
using namespace Microsoft::WRL;
using namespace Platform;

using namespace $ext_safeprojectname$;

// Helper to convert a length in device-independent pixels (DIPs) to a length in physical pixels.
inline float ConvertDipsToPixels(float dips, float dpi)
{
    static const float dipsPerInch = 96.0f;
    return floor(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

// Implementation of the IFrameworkViewSource interface, necessary to run our app.
ref class HelloTriangleApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
    virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView()
    {
        return ref new App();
    }
};

// The main function creates an IFrameworkViewSource for our app, and runs the app.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
    auto helloTriangleApplicationSource = ref new HelloTriangleApplicationSource();
    CoreApplication::Run(helloTriangleApplicationSource);
    return 0;
}

App::App() :
    mWindowClosed(false),
    mWindowVisible(true),
    mWindowWidth(0),
    mWindowHeight(0),
    mEglDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT),
    mEglSurface(EGL_NO_SURFACE),
    mCustomRenderSurfaceSize(0,0),
    mUseCustomRenderSurfaceSize(false)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
    // Register event handlers for app lifecycle. This example includes Activated, so that we
    // can make the CoreWindow active and start rendering on the window.
    applicationView->Activated += 
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

    // Logic for other event handlers could go here.
    // Information about the Suspending and Resuming event handlers can be found here:
    // http://msdn.microsoft.com/en-us/library/windows/apps/xaml/hh994930.aspx
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
    window->VisibilityChanged +=
        ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

    window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

    window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

#if !(WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
    // Disable all pointer visual feedback for better performance when touching.
    // This is not supported on Windows Phone applications.
    auto pointerVisualizationSettings = PointerVisualizationSettings::GetForCurrentView();
    pointerVisualizationSettings->IsContactFeedbackEnabled = false;
    pointerVisualizationSettings->IsBarrelButtonFeedbackEnabled = false;
#endif

    // The CoreWindow has been created, so EGL can be initialized.
    InitializeEGL(window);
    UpdateWindowSize(Size(window->Bounds.Width, window->Bounds.Height));
}

// Initializes scene resources
void App::Load(Platform::String^ entryPoint)
{
    RecreateRenderer();
}

void App::RecreateRenderer()
{
    if (!mTriangleRenderer)
    {
        mTriangleRenderer.reset(new HelloTriangleRenderer());
        mTriangleRenderer->UpdateWindowSize(mWindowWidth, mWindowHeight);
    }
}

// This method is called after the window becomes active.
void App::Run()
{
    while (!mWindowClosed)
    {
        if (mWindowVisible)
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);

            // Logic to update the scene could go here
            mTriangleRenderer->Draw();

            // The call to eglSwapBuffers might not be successful (e.g. due to Device Lost)
            // If the call fails, then we must reinitialize EGL and the GL resources.
            if (eglSwapBuffers(mEglDisplay, mEglSurface) != GL_TRUE)
            {
                mTriangleRenderer.reset(nullptr);
                CleanupEGL();

                InitializeEGL(CoreWindow::GetForCurrentThread());
                RecreateRenderer();
            }
        }
        else
        {
            CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
        }
    }

    CleanupEGL();
}

// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
}

// Application lifecycle event handler.
void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
    // Run() won't start until the CoreWindow is activated.
    CoreWindow::GetForCurrentThread()->Activate();
}

// Window event handlers.
void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
    mWindowVisible = args->Visible;
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
    mWindowClosed = true;
}

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
#if (WINAPI_FAMILY == WINAPI_FAMILY_PC_APP)
    // On Windows 8.1, apps are resized when they are snapped alongside other apps, or when the device is rotated.
    // The default framebuffer will be automatically resized when either of these occur.
    // In particular, on a 90 degree rotation, the default framebuffer's width and height will switch.
    UpdateWindowSize(args->Size);
#else if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
    // On Windows Phone 8.1, the window size changes when the device is rotated.
    // The default framebuffer will not be automatically resized when this occurs.
    // It is therefore up to the app to handle rotation-specific logic in its rendering code.
#endif
}

void App::InitializeEGL(CoreWindow^ window)
{
    const EGLint configAttributes[] = 
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };

    const EGLint displayAttributes[] =
    {
        // This can be used to configure D3D11. For example, EGL_PLATFORM_ANGLE_TYPE_D3D11_FL9_3_ANGLE could be used.
        // This would ask the graphics card to use D3D11 Feature Level 9_3 instead of Feature Level 11_0+.
        // On Windows Phone, this would allow the Phone Emulator to act more like the GPUs that are available on real Phone devices.
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_NONE,
    };

    const EGLint contextAttributes[] = 
    { 
        EGL_CONTEXT_CLIENT_VERSION, 2, 
        EGL_NONE
    };
    
    EGLConfig config = NULL;

    // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to pass in 'displayAttributes' to configure D3D11.
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!eglGetPlatformDisplayEXT)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get function eglGetPlatformDisplayEXT");
    }

    mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, displayAttributes);
    if (mEglDisplay == EGL_NO_DISPLAY)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get default EGL display");
    }

    if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to initialize EGL");
    }

    EGLint numConfigs = 0;
    if (eglGetConfigs(mEglDisplay, NULL, 0, &numConfigs) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get EGLConfig count");
    }

    if (eglChooseConfig(mEglDisplay, configAttributes, &config, 1, &numConfigs) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to choose first EGLConfig");
    }

    // Create a PropertySet and initialize with the EGLNativeWindowType.
    PropertySet^ surfaceCreationProperties = ref new PropertySet();
    surfaceCreationProperties->Insert(ref new String(EGLNativeWindowTypeProperty), window);

    //
    // A Custom render surface size can be specified by uncommenting the following lines.
    // The render surface will be automatically scaled to fit the entire window.  Using a
    // smaller sized render surface can result in a performance gain.
    //
    //mCustomRenderSurfaceSize = Size(800, 600);
    //mUseCustomRenderSurfaceSize = true;
    //surfaceCreationProperties->Insert(ref new String(EGLRenderSurfaceSizeProperty), PropertyValue::CreateSize(mCustomRenderSurfaceSize));
    //

    mEglSurface = eglCreateWindowSurface(mEglDisplay, config, reinterpret_cast<IInspectable*>(surfaceCreationProperties), NULL);
    if (mEglSurface == EGL_NO_SURFACE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL fullscreen surface");
    }

    mEglContext = eglCreateContext(mEglDisplay, config, EGL_NO_CONTEXT, contextAttributes);
    if (mEglContext == EGL_NO_CONTEXT)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL context");
    }

    if (eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to make fullscreen EGLSurface current");
    }
}

void App::CleanupEGL()
{
    if (mEglDisplay != EGL_NO_DISPLAY && mEglSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(mEglDisplay, mEglSurface);
        mEglSurface = EGL_NO_SURFACE;
    }

    if (mEglDisplay != EGL_NO_DISPLAY && mEglContext != EGL_NO_CONTEXT)
    {
        eglDestroyContext(mEglDisplay, mEglContext);
        mEglContext = EGL_NO_CONTEXT;
    }

    if (mEglDisplay != EGL_NO_DISPLAY)
    {
        eglTerminate(mEglDisplay);
        mEglDisplay = EGL_NO_DISPLAY;
    }
}

void App::UpdateWindowSize(Size size)
{
    Size pixelSize;
    // Use the dimensions of the custom render surface size if one was specified.
    if (mUseCustomRenderSurfaceSize)
    {
        // Render surface size is already in pixels
        pixelSize = mCustomRenderSurfaceSize;
    }
    else
    {
        DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
        pixelSize = Size(ConvertDipsToPixels(size.Width, currentDisplayInformation->LogicalDpi), ConvertDipsToPixels(size.Height, currentDisplayInformation->LogicalDpi));
    }

    mWindowWidth = static_cast<GLsizei>(pixelSize.Width);
    mWindowHeight = static_cast<GLsizei>(pixelSize.Height);

    // mTriangleRenderer might not have been initialized yet.
    if (mTriangleRenderer)
    {
        mTriangleRenderer->UpdateWindowSize(mWindowWidth, mWindowHeight);
    }
}