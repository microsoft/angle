//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <cstdint>
#include "com_utils.h"
#include "OSWindow.h"
#include <d3d11.h>

using namespace angle;

class EGLDirectRenderingD3D11 : public testing::TestWithParam<PlatformParameters>
{
  protected:
    EGLDirectRenderingD3D11()
        : mDisplay(EGL_NO_DISPLAY),
          mContext(EGL_NO_CONTEXT),
          mSurface(EGL_NO_SURFACE),
          mOffscreenSurfaceD3D11Texture(nullptr),
          mConfig(0),
          mOSWindow(nullptr),
          mWindowWidth(0)
    {
    }

    void SetUp() override
    {
        mOSWindow    = CreateOSWindow();
        mWindowWidth = 64;
        mOSWindow->initialize("EGLDirectRenderingD3D11", mWindowWidth, mWindowWidth);
    }

    void initializeEGL(bool enableDirectRendering)
    {
        int clientVersion = GetParam().majorVersion;

        PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT =
            reinterpret_cast<PFNEGLGETPLATFORMDISPLAYEXTPROC>(
                eglGetProcAddress("eglGetPlatformDisplayEXT"));
        EXPECT_TRUE(eglGetPlatformDisplayEXT != NULL);

        // Set up EGL Display
        EGLint displayAttribs[] = {EGL_PLATFORM_ANGLE_TYPE_ANGLE,
                                   GetParam().getRenderer(),
                                   EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE,
                                   GetParam().eglParameters.majorVersion,
                                   EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE,
                                   GetParam().eglParameters.majorVersion,
                                   EGL_PLATFORM_ANGLE_EXPERIMENTAL_DIRECT_RENDERING,
                                   enableDirectRendering ? EGL_TRUE : EGL_FALSE,
                                   EGL_NONE};
        mDisplay =
            eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, EGL_DEFAULT_DISPLAY, displayAttribs);
        EXPECT_TRUE(mDisplay != EGL_NO_DISPLAY);
        EXPECT_TRUE(eglInitialize(mDisplay, NULL, NULL) != EGL_FALSE);

        // Choose the EGL config
        EGLint numConfigs;
        EGLint configAttribs[] =
        {
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_RENDERABLE_TYPE, clientVersion == 3 ? EGL_OPENGL_ES3_BIT : EGL_OPENGL_ES2_BIT,
            EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
            EGL_NONE
        };
        EXPECT_TRUE(eglChooseConfig(mDisplay, configAttribs, &mConfig, 1, &numConfigs) !=
                    EGL_FALSE);
        EXPECT_TRUE(numConfigs == 1);

        // Set up the EGL context
        EGLint contextAttribs[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, clientVersion,
            EGL_NONE
        };
        mContext = eglCreateContext(mDisplay, mConfig, NULL, contextAttribs);
        EXPECT_TRUE(mContext != EGL_NO_CONTEXT);
    }

    void createWindowSurface()
    {
        mSurface = eglCreateWindowSurface(mDisplay, mConfig, mOSWindow->getNativeWindow(), nullptr);
    }

    void createPbufferFromClientBufferSurface()
    {
        EGLAttrib device      = 0;
        EGLAttrib angleDevice = 0;

        PFNEGLQUERYDISPLAYATTRIBEXTPROC queryDisplayAttribEXT;
        PFNEGLQUERYDEVICEATTRIBEXTPROC queryDeviceAttribEXT;

        const char *extensionString =
            static_cast<const char *>(eglQueryString(mDisplay, EGL_EXTENSIONS));
        ASSERT_TRUE(strstr(extensionString, "EGL_EXT_device_query"));

        queryDisplayAttribEXT =
            (PFNEGLQUERYDISPLAYATTRIBEXTPROC)eglGetProcAddress("eglQueryDisplayAttribEXT");
        queryDeviceAttribEXT =
            (PFNEGLQUERYDEVICEATTRIBEXTPROC)eglGetProcAddress("eglQueryDeviceAttribEXT");

        ASSERT_EGL_TRUE(queryDisplayAttribEXT(mDisplay, EGL_DEVICE_EXT, &angleDevice));
        ASSERT_EGL_TRUE(queryDeviceAttribEXT(reinterpret_cast<EGLDeviceEXT>(angleDevice),
                                             EGL_D3D11_DEVICE_ANGLE, &device));
        ID3D11Device *d3d11Device = reinterpret_cast<ID3D11Device *>(device);

        D3D11_TEXTURE2D_DESC textureDesc = {0};
        textureDesc.Width                = mWindowWidth;
        textureDesc.Height               = mWindowWidth;
        textureDesc.Format               = DXGI_FORMAT_B8G8R8A8_UNORM;
        textureDesc.MipLevels            = 1;
        textureDesc.ArraySize            = 1;
        textureDesc.SampleDesc.Count     = 1;
        textureDesc.SampleDesc.Quality   = 0;
        textureDesc.Usage                = D3D11_USAGE_DEFAULT;
        textureDesc.BindFlags            = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.CPUAccessFlags       = 0;
        textureDesc.MiscFlags            = D3D11_RESOURCE_MISC_SHARED;

        ASSERT_TRUE(SUCCEEDED(
            d3d11Device->CreateTexture2D(&textureDesc, nullptr, &mOffscreenSurfaceD3D11Texture)));

        IDXGIResource *dxgiResource =
            DynamicCastComObject<IDXGIResource>(mOffscreenSurfaceD3D11Texture);
        ASSERT_NE(nullptr, dxgiResource);

        HANDLE sharedHandle = 0;
        ASSERT_TRUE(SUCCEEDED(dxgiResource->GetSharedHandle(&sharedHandle)));
        SafeRelease(dxgiResource);

        EGLint pBufferAttributes[] =
        {
            EGL_WIDTH, mWindowWidth,
            EGL_HEIGHT, mWindowWidth,
            EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
            EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
            EGL_NONE
        };

        mSurface = eglCreatePbufferFromClientBuffer(mDisplay, EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE,
                                                    sharedHandle, mConfig, pBufferAttributes);
        ASSERT_NE(EGL_NO_SURFACE, mSurface);
    }

    void makeCurrent() { EXPECT_TRUE(eglMakeCurrent(mDisplay, mSurface, mSurface, mContext)); }

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

        SafeRelease(mOffscreenSurfaceD3D11Texture);

        ASSERT_TRUE(mSurface == EGL_NO_SURFACE && mContext == EGL_NO_CONTEXT);
    }

    void drawQuadUsingGL()
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

        m2DProgram                = CompileProgram(vertexShaderSource, fragmentShaderSource2D);
        mTexture2DUniformLocation = glGetUniformLocation(m2DProgram, "tex");

        uint8_t textureInitData[16] =
        {
            255,   0,   0, 255,  // Red
              0, 255,   0, 255,  // Green
              0,   0, 255, 255,  // Blue
            255, 255,   0, 255   // Red + Green
        };

        // Create a simple RGBA texture
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     textureInitData);
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

        glDeleteProgram(m2DProgram);
    }

    void checkPixelsUsingGL()
    {
        EXPECT_PIXEL_EQ(               0,                0, 255,   0,   0, 255); // Red
        EXPECT_PIXEL_EQ(mWindowWidth - 1,                0,   0, 255,   0, 255); // Green
        EXPECT_PIXEL_EQ(               0, mWindowWidth - 1,   0,   0, 255, 255); // Blue
        EXPECT_PIXEL_EQ(mWindowWidth - 1, mWindowWidth - 1, 255, 255,   0, 255); // Red + green
    }

    void checkPixelsUsingD3D(bool usingDirectRendering)
    {
        ASSERT_NE(nullptr, mOffscreenSurfaceD3D11Texture);

        D3D11_TEXTURE2D_DESC textureDesc = {0};
        ID3D11Device *device;
        ID3D11DeviceContext *context;
        mOffscreenSurfaceD3D11Texture->GetDesc(&textureDesc);
        mOffscreenSurfaceD3D11Texture->GetDevice(&device);
        device->GetImmediateContext(&context);
        ASSERT_NE(nullptr, device);
        ASSERT_NE(nullptr, context);

        textureDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_READ;
        textureDesc.Usage           = D3D11_USAGE_STAGING;
        textureDesc.BindFlags       = 0;
        textureDesc.MiscFlags       = 0;
        ID3D11Texture2D *cpuTexture = nullptr;
        ASSERT_TRUE(SUCCEEDED(device->CreateTexture2D(&textureDesc, nullptr, &cpuTexture)));

        context->CopyResource(cpuTexture, mOffscreenSurfaceD3D11Texture);

        D3D11_MAPPED_SUBRESOURCE mappedSubresource;
        context->Map(cpuTexture, 0, D3D11_MAP_READ, 0, &mappedSubresource);
        ASSERT_EQ(mWindowWidth * 4, mappedSubresource.RowPitch);
        ASSERT_EQ(mWindowWidth * mWindowWidth * 4, mappedSubresource.DepthPitch);

        uint8_t *byteData = reinterpret_cast<uint8_t *>(mappedSubresource.pData);

        // Note that the texture is in BGRA format
        std::vector<uint8_t> expectedTopLeftPixel =     {   0,   0, 255, 255 }; // Red
        std::vector<uint8_t> expectedTopRightPixel =    {   0, 255,   0, 255 }; // Green
        std::vector<uint8_t> expectedBottomLeftPixel =  { 255,   0,   0, 255 }; // Blue
        std::vector<uint8_t> expectedBottomRightPixel = {   0, 255, 255, 255 }; // Red + Green

        if (usingDirectRendering)
        {
            // Invert the expected values
            std::vector<uint8_t> tempTopLeft  = expectedTopLeftPixel;
            std::vector<uint8_t> tempTopRight = expectedTopRightPixel;
            expectedTopLeftPixel              = expectedBottomLeftPixel;
            expectedTopRightPixel             = expectedBottomRightPixel;
            expectedBottomLeftPixel           = tempTopLeft;
            expectedBottomRightPixel          = tempTopRight;
        }

        EXPECT_EQ(expectedTopLeftPixel[0], byteData[0]);
        EXPECT_EQ(expectedTopLeftPixel[1], byteData[1]);
        EXPECT_EQ(expectedTopLeftPixel[2], byteData[2]);
        EXPECT_EQ(expectedTopLeftPixel[3], byteData[3]);

        EXPECT_EQ(expectedTopRightPixel[0], byteData[(mWindowWidth - 1) * 4 + 0]);
        EXPECT_EQ(expectedTopRightPixel[1], byteData[(mWindowWidth - 1) * 4 + 1]);
        EXPECT_EQ(expectedTopRightPixel[2], byteData[(mWindowWidth - 1) * 4 + 2]);
        EXPECT_EQ(expectedTopRightPixel[3], byteData[(mWindowWidth - 1) * 4 + 3]);

        EXPECT_EQ(expectedBottomLeftPixel[0], byteData[(mWindowWidth - 1) * 4 * mWindowWidth + 0]);
        EXPECT_EQ(expectedBottomLeftPixel[1], byteData[(mWindowWidth - 1) * 4 * mWindowWidth + 1]);
        EXPECT_EQ(expectedBottomLeftPixel[2], byteData[(mWindowWidth - 1) * 4 * mWindowWidth + 2]);
        EXPECT_EQ(expectedBottomLeftPixel[3], byteData[(mWindowWidth - 1) * 4 * mWindowWidth + 3]);

        EXPECT_EQ(expectedBottomRightPixel[0],
                  byteData[(mWindowWidth - 1) * 4 * mWindowWidth + (mWindowWidth - 1) * 4 + 0]);
        EXPECT_EQ(expectedBottomRightPixel[1],
                  byteData[(mWindowWidth - 1) * 4 * mWindowWidth + (mWindowWidth - 1) * 4 + 1]);
        EXPECT_EQ(expectedBottomRightPixel[2],
                  byteData[(mWindowWidth - 1) * 4 * mWindowWidth + (mWindowWidth - 1) * 4 + 2]);
        EXPECT_EQ(expectedBottomRightPixel[3],
                  byteData[(mWindowWidth - 1) * 4 * mWindowWidth + (mWindowWidth - 1) * 4 + 3]);

        context->Unmap(cpuTexture, 0);
        SafeRelease(cpuTexture);
        SafeRelease(device);
        SafeRelease(context);
    }

    EGLDisplay mDisplay;
    EGLContext mContext;
    EGLSurface mSurface;
    ID3D11Texture2D *mOffscreenSurfaceD3D11Texture;
    EGLConfig mConfig;
    OSWindow *mOSWindow;
    GLint mWindowWidth;
};

// Test that rendering basic content onto a window surface when direct rendering
// is enabled works as expected
TEST_P(EGLDirectRenderingD3D11, WindowEnableDirectRendering)
{
    initializeEGL(true);
    createWindowSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
}

// Test that rendering basic content onto a client buffer surface when direct rendering
// is enabled works as expected, and is also oriented the correct way around
TEST_P(EGLDirectRenderingD3D11, ClientBufferEnableDirectRendering)
{
    initializeEGL(true);
    createPbufferFromClientBufferSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
    checkPixelsUsingD3D(true);
}

// Test that rendering basic content onto a window surface when direct rendering
// is disabled works as expected
TEST_P(EGLDirectRenderingD3D11, WindowDisableDirectRendering)
{
    initializeEGL(false);
    createWindowSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
}

// Test that rendering basic content onto a client buffer surface when direct rendering
// is disabled works as expected, and is also oriented the correct way around
TEST_P(EGLDirectRenderingD3D11, ClientBufferDisableDirectRendering)
{
    initializeEGL(false);
    createPbufferFromClientBufferSurface();
    makeCurrent();

    drawQuadUsingGL();

    checkPixelsUsingGL();
    checkPixelsUsingD3D(false);
}

ANGLE_INSTANTIATE_TEST(EGLDirectRenderingD3D11, ES2_D3D11(), ES2_D3D11_FL9_3());