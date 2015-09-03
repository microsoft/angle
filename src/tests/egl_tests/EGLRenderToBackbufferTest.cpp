//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <cstdint>

#include "OSWindow.h"

using namespace angle;

class EGLRenderToBackbufferTest : public testing::TestWithParam<PlatformParameters>
{
  protected:

    void SetUp() override
    {
        mOSWindow = CreateOSWindow();
        mWindowWidth = 64;
        mOSWindow->initialize("EGLSurfaceTest", mWindowWidth, mWindowWidth);
    }

    void initializeEGL(bool allowRTBB, bool enableRTBB, bool expectSurface)
    {
        int clientVersion = GetParam().majorVersion;

        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT = reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(eglGetProcAddress("eglGetPlatformDisplayEXT"));
        EXPECT_TRUE(eglGetPlatformDisplayEXT != NULL);

        // Set up EGL Display
        EGLint dispattrs[] =
        {
            EGL_PLATFORM_ANGLE_TYPE_ANGLE, GetParam().getRenderer(),
            EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, GetParam().eglParameters.majorVersion,
            EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, GetParam().eglParameters.majorVersion,
            EGL_ANGLE_DISPLAY_ALLOW_RENDER_TO_BACK_BUFFER, allowRTBB ? EGL_TRUE : EGL_FALSE,
            EGL_NONE
        };
        mDisplay = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, dispattrs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_TRUE(eglInitialize(mDisplay, NULL, NULL) != EGL_FALSE);

        // Choose the EGL config
        EGLint ncfg;
        EGLConfig config;
        EGLint cfgattrs[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_RENDERABLE_TYPE, clientVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_NONE
        };
        EXPECT_TRUE(eglChooseConfig(mDisplay, cfgattrs, &config, 1, &ncfg) != EGL_FALSE);
        EXPECT_TRUE(ncfg == 1);

        // Set up the EGL context
        EGLint ctxattrs[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, clientVersion,
            EGL_NONE
        };
        mContext = eglCreateContext(mDisplay, config, NULL, ctxattrs);
        EXPECT_TRUE(mContext != EGL_NO_CONTEXT);

        // Set up the EGL surface
        const EGLint surfaceAttributes[] =
        {
            EGL_ANGLE_SURFACE_RENDER_TO_BACK_BUFFER, enableRTBB ? EGL_TRUE : EGL_FALSE,
            EGL_NONE
        };
        mSurface = eglCreateWindowSurface(mDisplay, config, mOSWindow->getNativeWindow(), surfaceAttributes);
        ASSERT_EQ(expectSurface, mSurface != EGL_NO_SURFACE);

        if (expectSurface)
        {
            EGLBoolean makeCurrent = eglMakeCurrent(mDisplay, mSurface, mSurface, mContext);
            EXPECT_TRUE(makeCurrent == EGL_TRUE);
        }
    }

    void TearDown() override
    {
        if (mDisplay != EGL_NO_DISPLAY)
        {
            eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

            if (mSurface != EGL_NO_SURFACE)
            {
                eglDestroySurface(mDisplay, mSurface);
                mSurface = EGL_NO_SURFACE;
            }

            if (mContext != EGL_NO_CONTEXT)
            {
                eglDestroyContext(mDisplay, mContext);
                mContext = EGL_NO_CONTEXT;
            }

            eglTerminate(mDisplay);
            mDisplay = EGL_NO_DISPLAY;
        }

        mOSWindow->destroy();
        SafeDelete(mOSWindow);

        ASSERT_TRUE(mSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    void drawAndCheckQuad()
    {
        GLuint m2DProgram;
        GLint mTexture2DUniformLocation;

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = vec4(position.xy, 0.0, 1.0);
                texcoord = (position.xy * 0.5) + 0.5;
            }
        );

        const std::string fragmentShaderSource2D = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        );

        m2DProgram = CompileProgram(vertexShaderSource, fragmentShaderSource2D);
        mTexture2DUniformLocation = glGetUniformLocation(m2DProgram, "tex");

        byte textureInitData[16] =
        {
            255, 0, 0, 255,  // Red
            0, 255, 0, 255,  // Green
            0, 0, 255, 255,  // Blue
            255, 255, 0, 255 // Red + Green
        };

        // Create a simple RGBA texture
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureInitData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        EXPECT_GL_NO_ERROR();

        // Draw a quad using the texture
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(m2DProgram);
        glUniform1i(mTexture2DUniformLocation, 0);

        GLint positionLocation = glGetAttribLocation(m2DProgram, "position");
        glUseProgram(m2DProgram);
        const GLfloat vertices[] =
        {
            -1.0f,  1.0f, 0.5f,
            -1.0f, -1.0f, 0.5f,
             1.0f, -1.0f, 0.5f,

            -1.0f,  1.0f, 0.5f,
             1.0f, -1.0f, 0.5f,
             1.0f,  1.0f, 0.5f,
        };

        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        glEnableVertexAttribArray(positionLocation);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_GL_NO_ERROR();

        // Check that it drew as expected
        EXPECT_PIXEL_EQ(               0,                0, 255,   0,   0, 255);
        EXPECT_PIXEL_EQ(mWindowWidth - 1,                0,   0, 255,   0, 255);
        EXPECT_PIXEL_EQ(               0, mWindowWidth - 1,   0,   0, 255, 255);
        EXPECT_PIXEL_EQ(mWindowWidth - 1, mWindowWidth - 1, 255, 255,   0, 255);
        eglSwapBuffers(mDisplay, mSurface);

        glDeleteProgram(m2DProgram);
    }

    EGLDisplay mDisplay;
    EGLContext mContext;
    EGLSurface mSurface;
    OSWindow *mOSWindow;
    GLint mWindowWidth;
};

TEST_P(EGLRenderToBackbufferTest, AllowAndEnable)
{
    initializeEGL(true, true, true);

    drawAndCheckQuad();
}

TEST_P(EGLRenderToBackbufferTest, AllowAndDisable)
{
    initializeEGL(true, false, true);

    drawAndCheckQuad();
}

TEST_P(EGLRenderToBackbufferTest, DisallowAndEnable)
{
    initializeEGL(false, true, false);

    // Disallowing RTBB but enabling it shouldn't work
    ASSERT(mSurface == EGL_NO_SURFACE);
}

TEST_P(EGLRenderToBackbufferTest, DisallowAndDisable)
{
    initializeEGL(false, false, true);

    drawAndCheckQuad();
}

ANGLE_INSTANTIATE_TEST(EGLRenderToBackbufferTest, ES2_D3D11(), ES2_D3D11_FL9_3());