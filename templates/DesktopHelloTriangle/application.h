#pragma once

#include "pch.h"

#include "HelloTriangleRenderer.h"

namespace $safeprojectname$
{
    class Application
    {
    public:
        Application(const std::wstring& windowTitle);

        int Run();

        void OnWindowSizeChanged(int width, int height);
        void OnWindowClose();

    private:
        bool InitializeEGL();
        void CleanupEGL();

        bool InitializeWindow(int width, int height);
        void CleanupWindow();

        void Step(double deltaTime, double elapsedTime);

        EGLDisplay mEGLDisplay;
        EGLContext mEGLContext;
        EGLSurface mEGLSurface;
        EGLNativeWindowType mNativeWindow;

        std::unique_ptr<HelloTriangleRenderer> mTriangleRenderer;

        std::wstring mWindowTitle;
        bool mIsRunning;

    };
}
