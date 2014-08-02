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

void EGLWrapper::InitializeOffscreenSurfaceEGL(EGLNativeDisplayType displayId)
{
    EGLint configAttribList[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 0,
        EGL_NONE
    };

    EGLint pBufferAttribList[] = {
        EGL_WIDTH, 64,
        EGL_HEIGHT, 64,
        EGL_LARGEST_PBUFFER, EGL_FALSE,
        EGL_NONE
    };

    EGLint numConfigs = 0;
    EGLint majorVersion = 1;
    EGLint minorVersion = 0;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLSurface surface = EGL_NO_SURFACE;
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

    surface = eglCreatePbufferSurface(display, config, pBufferAttribList);
    if (surface == EGL_NO_SURFACE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to create EGL surface");
    }

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
    {
        throw Exception::CreateException(E_FAIL, L"Failed to make display and context current.");
    }

    mDisplay = display;
    mContext = context;
    mSurface = surface;
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

/*
void EGLWrapper::ForceRendererCreation()
{
    GLuint offscreenFramebuffer;
    GLuint offscreenTexture;

    glGenFramebuffers(1, &offscreenFramebuffer);
    glGenTextures(1, &offscreenTexture);

    glBindTexture(GL_TEXTURE_2D, offscreenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, offscreenTexture, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw Exception::CreateException(E_FAIL, L"Framebuffer is not complete.");
    }

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glDeleteFramebuffers(1, &offscreenFramebuffer);
    glDeleteTextures(1, &offscreenTexture);
}
*/