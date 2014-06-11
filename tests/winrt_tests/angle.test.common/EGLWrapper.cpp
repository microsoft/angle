//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "EGLWrapper.h"

using namespace Platform;

EGLWrapper::EGLWrapper() 
    : mDisplay( EGL_NO_DISPLAY),
      mSurface( EGL_NO_SURFACE),
      mContext( EGL_NO_CONTEXT)
{
}

void EGLWrapper::InitializeSurfacelessEGL(EGLNativeDisplayType displayId)
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
    EGLConfig config = nullptr;
    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE, EGL_NONE };

    display = eglGetDisplay(displayId);
    if (display == EGL_NO_DISPLAY)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get default EGL display.");
    }

    if (eglInitialize(display, &majorVersion, &minorVersion) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to initialize EGL.");
    }

    if (eglGetConfigs(display, NULL, 0, &numConfigs) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to get EGLConfig count.");
    }

    if (eglChooseConfig(display, configAttribList, &config, 1, &numConfigs) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to choose first EGLConfig count.");
    }

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL context");
    }

    if (eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to make display and context current.");
    }

    mDisplay = display;
    mContext = context;
}

void EGLWrapper::CleanupEGL()
{
    if (mDisplay != EGL_NO_DISPLAY && mSurface != EGL_NO_SURFACE)
    {
        eglDestroySurface(mDisplay, mSurface);
        mSurface = EGL_NO_SURFACE;
    }

    if (mDisplay != EGL_NO_DISPLAY && mContext != EGL_NO_CONTEXT)
    {
        eglDestroyContext(mDisplay, mContext);
        mContext = EGL_NO_CONTEXT;
    }

    if (mDisplay != EGL_NO_DISPLAY)
    {
        eglTerminate(mDisplay);
        mDisplay = EGL_NO_DISPLAY;
    }
}