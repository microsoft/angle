#include "pch.h"

using namespace Platform;

OpenGLES::OpenGLES() :
    mEglConfig(nullptr),
    mEglDisplay(EGL_NO_DISPLAY),
    mEglContext(EGL_NO_CONTEXT)
{
    Initialize();
}

OpenGLES::~OpenGLES()
{
    Cleanup();
}

void OpenGLES::Initialize()
{
    EGLint configAttribList[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    EGLint numConfigs = 0;
    EGLint majorVersion = 1;
    EGLint minorVersion = 0;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
    EGLConfig config = nullptr;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE, EGL_NONE };

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get default EGL display");
    }

    if (eglInitialize(display, &majorVersion, &minorVersion) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to initialize EGL");
    }

    if (eglGetConfigs(display, NULL, 0, &numConfigs) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get EGLConfig count");
    }

    if (eglChooseConfig(display, configAttribList, &config, 1, &numConfigs) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to choose first EGLConfig count");
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL context");
    }

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to make EGLSurface current");
    }

    mEglDisplay = display;
    mEglContext = context;
    mEglConfig = config;
}

void OpenGLES::Cleanup()
{
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

void OpenGLES::Reset()
{
    Cleanup();
    Initialize();
}

EGLSurface OpenGLES::CreateSurface(Windows::UI::Xaml::Controls::SwapChainPanel^ panel)
{
    if (!panel)
    {
        throw Exception::CreateException(E_INVALIDARG, L"SwapChainPanel parameter is invalid");
    }

    EGLSurface surface = EGL_NO_SURFACE;
    EGLint surfaceAttribList[] = {
        EGL_NONE, EGL_NONE
    };

    surface = eglCreateWindowSurface(mEglDisplay, mEglConfig, reinterpret_cast<IInspectable*>(panel), surfaceAttribList);
    if (surface == EGL_NO_SURFACE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL surface");
    }

    return surface;
}

void OpenGLES::DestroySurface(const EGLSurface surface)
{
    if (mEglDisplay != EGL_NO_DISPLAY && surface != EGL_NO_SURFACE)
    {
        eglDestroySurface(mEglDisplay, surface);
    }
}

void OpenGLES::MakeCurrent(const EGLSurface surface)
{
    if (eglMakeCurrent(mEglDisplay, surface, surface, mEglContext) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to make EGLSurface current");
    }
}

EGLBoolean OpenGLES::SwapBuffers(const EGLSurface surface)
{
    return (eglSwapBuffers(mEglDisplay, surface));
}
