#include "pch.h"
#include "application.h"

using namespace $safeprojectname$;

// Used to initialize our app and run it.
int main(int argc, char **argv)
{
    Application app = Application(L"$safeprojectname$");
    return app.Run();
}

Application::Application(const std::wstring& windowTitle)
    : mEglDisplay(EGL_NO_DISPLAY),
      mEglContext(EGL_NO_CONTEXT),
      mEglSurface(EGL_NO_SURFACE),
      mNativeWindow(0),
      mWindowTitle(windowTitle),
      mIsRunning(false)
{
}

int Application::Run()
{
    if (!InitializeWindow(640, 480))
    {
        OutputDebugStringW(L"Failed to initialize the window. Exiting.");
        CleanupWindow();
        return -1;
    }

    if (!InitializeEGL())
    {
        OutputDebugStringW(L"Failed to initialize EGL. Exiting.");
        CleanupEGL();
        CleanupWindow();
        return -1;
    }

    mTriangleRenderer.reset(new HelloTriangleRenderer());
    if (!mTriangleRenderer->Initialize())
    {
        OutputDebugStringW(L"Failed to initialize the triangle renderer. Exiting.");
        CleanupEGL();
        CleanupWindow();
        return -1;
    }

    mIsRunning = true;

    LONGLONG startTime;
    LONGLONG currentTime;
    LONGLONG frequency;

    LARGE_INTEGER qpfFrequency;
    QueryPerformanceFrequency(&qpfFrequency);
    frequency = qpfFrequency.QuadPart;

    LARGE_INTEGER qpcCurrentTime;
    QueryPerformanceCounter(&qpcCurrentTime);
    startTime = qpcCurrentTime.QuadPart;

    double prevElapsedTime = 0.0;

    while (mIsRunning)
    {
        QueryPerformanceCounter(&qpcCurrentTime);
        currentTime = qpcCurrentTime.QuadPart;

        double elapsedTime = static_cast<double>(currentTime - startTime) / frequency;
        double deltaTime = elapsedTime - prevElapsedTime;
        
        Step(deltaTime, elapsedTime);

        mTriangleRenderer->Draw();
        
        // The call to eglSwapBuffers might not be successful (e.g. due to Device Lost)
        // If the call fails, then we must reinitialize EGL and the GL resources.
        if (eglSwapBuffers(mEglDisplay, mEglSurface) != GL_TRUE)
        {
            OutputDebugStringW(L"Call to eglSwapBuffers failed.");
            mTriangleRenderer.reset(nullptr);
            CleanupEGL();

            if (!InitializeEGL())
            {
                OutputDebugStringW(L"Failed to reinitialize EGL. Exiting.");
                break;
            }

            mTriangleRenderer.reset(new HelloTriangleRenderer());
            if (!mTriangleRenderer->Initialize())
            {
                OutputDebugStringW(L"Failed to reinitialize the triangle renderer. Exiting.");
                break;
            }
        }

        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        
        prevElapsedTime = elapsedTime;
    }

    CleanupEGL();
    CleanupWindow();
    return 0;
}

bool Application::InitializeEGL()
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

    const EGLint surfaceAttributes[] =
    {
        EGL_NONE
    };

    const EGLint contextAttibutes[] =
    {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    const EGLint displayAttributes[] =
    {
        EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_NONE,
    };

    EGLConfig config = 0;

    // eglGetPlatformDisplayEXT is an alternative to eglGetDisplay. It allows us to specifically request D3D11 instead of D3D9.
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
    if (!eglGetPlatformDisplayEXT)
    {
         OutputDebugStringW(L"Failed to get function eglGetPlatformDisplayEXT");
    }

    mEglDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, mHDC, displayAttributes);
    if (mEglDisplay == EGL_NO_DISPLAY)
    {
        OutputDebugStringW(L"Failed to get requested EGL display");
        CleanupEGL();
        return false;
    }

    if (eglInitialize(mEglDisplay, NULL, NULL) == EGL_FALSE)
    {
        OutputDebugStringW(L"Failed to initialize EGL");
        CleanupEGL();
        return false;
    }

    EGLint numConfigs;
    if ((eglChooseConfig(mEglDisplay, configAttributes, &config, 1, &numConfigs) == EGL_FALSE) || (numConfigs == 0))
    {
        OutputDebugStringW(L"Failed to choose first EGLConfig");
        CleanupEGL();
        return false;
    }

    mEglSurface = eglCreateWindowSurface(mEglDisplay, config, mNativeWindow, surfaceAttributes);
    if (mEglSurface == EGL_NO_SURFACE)
    {
        OutputDebugStringW(L"Failed to create EGL fullscreen surface");
        CleanupEGL();
        return false;
    }

    if (eglGetError() != EGL_SUCCESS)
    {
        OutputDebugStringW(L"eglGetError has reported an error");
        CleanupEGL();
        return false;
    }

    mEglContext = eglCreateContext(mEglDisplay, config, NULL, contextAttibutes);
    if (mEglContext == EGL_NO_CONTEXT)
    {
        OutputDebugStringW(L"Failed to create EGL context");
        CleanupEGL();
        return false;
    }

    if (eglMakeCurrent(mEglDisplay, mEglSurface, mEglSurface, mEglContext) == EGL_FALSE)
    {
        OutputDebugStringW(L"Failed to make EGLSurface current");
        CleanupEGL();
        return false;
    }

    return true;
}

void Application::CleanupEGL()
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
        eglMakeCurrent(mEglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(mEglDisplay);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    Application *application = (Application*)(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (application)
    {
        switch (message)
        {
        case WM_DESTROY:
        case WM_CLOSE:
            {
                application->OnWindowClose();
                break;
            }

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
            {
                if (wParam == VK_ESCAPE)
                {
                    // If the escape key is pressed, then close the application.
                    application->OnWindowClose();
                }

                break;
            }

        case WM_SIZE:
            {
                RECT winRect;
                GetClientRect(hWnd, &winRect);

                POINT topLeft;
                topLeft.x = winRect.left;
                topLeft.y = winRect.top;
                ClientToScreen(hWnd, &topLeft);

                POINT botRight;
                botRight.x = winRect.right;
                botRight.y = winRect.bottom;
                ClientToScreen(hWnd, &botRight);

                application->OnWindowSizeChanged(botRight.x - topLeft.x, botRight.y - topLeft.y);

                break;
            }
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

bool Application::InitializeWindow(int width, int height)
{
    WNDCLASSEXW windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEXW);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hbrBackground = 0;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = mWindowTitle.c_str();
    if (!RegisterClassExW(&windowClass))
    {
        return false;
    }

    DWORD style = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_SYSMENU;
    DWORD extendedStyle = WS_EX_APPWINDOW;

    RECT sizeRect = { 0, 0, width, height };
    AdjustWindowRectEx(&sizeRect, style, false, extendedStyle);

    mNativeWindow = CreateWindowExW(extendedStyle, mWindowTitle.c_str(), mWindowTitle.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
        sizeRect.right - sizeRect.left, sizeRect.bottom - sizeRect.top, NULL, NULL,
        GetModuleHandle(NULL), this);

    SetWindowLongPtrW(mNativeWindow, GWLP_USERDATA, reinterpret_cast<LONGLONG>(this));

    ShowWindow(mNativeWindow, SW_SHOW);

    mHDC = GetDC(mNativeWindow);
    if (!mHDC)
    {
        CleanupWindow();
        return false;
    }

    return true;
}

void Application::CleanupWindow()
{
    if (mHDC)
    {
        ReleaseDC(mNativeWindow, mHDC);
        mHDC = 0;
    }

    if (mNativeWindow)
    {
        DestroyWindow(mNativeWindow);
        mNativeWindow = 0;
    }

    UnregisterClassW(mWindowTitle.c_str(), NULL);
}

void Application::Step(double deltaTime, double elapsedTime)
{
    // Logic to update your scene could go here.
}

void Application::OnWindowSizeChanged(int width, int height)
{
    // Notify the triangle renderer that the Window size has changed.
    mTriangleRenderer->OnWindowSizeChanged(width, height);
}

void Application::OnWindowClose()
{
    mIsRunning = false;
}