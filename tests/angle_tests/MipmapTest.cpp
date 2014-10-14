#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D11_FL9_3>, TFT<Gles::Two, Rend::WARP>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(MipmapTest, TestFixtureTypes);

template<typename T>
class MipmapTest : public ANGLETest
{
protected:
    MipmapTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        // Vertex Shader source
        const std::string vs = SHADER_SOURCE
        (
            attribute vec4 aPosition;
            attribute vec2 aTexCoord;
            varying vec2 vTexCoord;

            void main()
            {
                gl_Position = aPosition;
                vTexCoord = aTexCoord;
            }
        );

        // Fragment Shader source
        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;

            uniform sampler2D uTexture;
            varying vec2 vTexCoord;

            void main()
            {
                gl_FragColor = texture2D(uTexture, vTexCoord);
            }
        );

        mProgram = CompileProgram(vs, fs);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformPosition = glGetUniformLocation(mProgram, "uTexture");
        mPositionAttributePosition = glGetAttribLocation(mProgram, "aPosition");
        mTexCoordAttributePosition = glGetAttribLocation(mProgram, "aTexCoord");

        glGenFramebuffers(1, &mOffscreenFramebuffer);
        glGenTextures(1, &mOffscreenTexture);

        // Initialize the texture to be empty, and don't use mips.
        glBindTexture(GL_TEXTURE_2D, mOffscreenTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        ASSERT_GL_NO_ERROR();

        // Bind the texture to the offscreen framebuffer's color buffer.
        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mOffscreenTexture, 0); 
        ASSERT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GL_FRAMEBUFFER_COMPLETE);

        mLevelZeroBlueInitData =  createRGBInitData(getWindowWidth(),       getWindowHeight(),       0,   0,   255); // Blue
        mLevelZeroWhiteInitData = createRGBInitData(getWindowWidth(),       getWindowHeight(),       255, 255, 255); // White
        mLevelOneInitData =       createRGBInitData((getWindowWidth() / 2), (getWindowHeight() / 2), 0,   255, 0);   // Green
        mLevelTwoInitData =       createRGBInitData((getWindowWidth() / 4), (getWindowHeight() / 4), 255, 0,   0);   // Red
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);
        glDeleteFramebuffers(1, &mOffscreenFramebuffer);
        glDeleteFramebuffers(1, &mOffscreenTexture);

        delete mLevelZeroBlueInitData;
        delete mLevelZeroWhiteInitData;
        delete mLevelOneInitData;
        delete mLevelTwoInitData;

        ANGLETest::TearDown();
    }

    GLubyte* createRGBInitData(GLint width, GLint height, GLint r, GLint g, GLint b)
    {
        GLubyte* data = new GLubyte[3 * width * height];

        for (int i = 0; i < width * height; i+=1)
        {
            data[3 * i + 0] = r;
            data[3 * i + 1] = g;
            data[3 * i + 2] = b;
        }

        return data;
    }

    void ClearAndDrawTexturedQuad(GLuint texture, GLsizei viewportWidth, GLsizei viewportHeight)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, viewportWidth, viewportHeight);

        ASSERT_GL_NO_ERROR();

        GLfloat vertexLocations[] =
        {
            -1.0f,  1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
        };

        GLfloat vertexTexCoords[] =
        {
            0.0f, 1.0f,
            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f,
        };

        glUseProgram(mProgram);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(mTextureUniformPosition, 0);

        glVertexAttribPointer(mPositionAttributePosition, 3, GL_FLOAT, GL_FALSE, 0, vertexLocations);
        glEnableVertexAttribArray(mPositionAttributePosition);

        glVertexAttribPointer(mTexCoordAttributePosition, 2, GL_FLOAT, GL_FALSE, 0, vertexTexCoords);
        glEnableVertexAttribArray(mTexCoordAttributePosition);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // A base method for some tests below.
    void RenderAfterGenerateMipmapBase(bool bUseMipmapsImmediatelyAfterClearingLevel0)
    {
        // Bind the offscreen texture/framebuffer.
        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);

        // Clear the texture to blue.
        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // From now on, default clear color is black.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
        // Now, draw the texture to a quad that's the same size as the texture. This draws to the default framebuffer. 
        // The quad should be blue.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
    
        // Now go back to the texture, and generate mips on it.
        glGenerateMipmap(GL_TEXTURE_2D);
        ASSERT_GL_NO_ERROR();

        // Now try rendering the textured quad again. Note: we've not told GL to use the generated mips.
        // The quad should be blue.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

        // Now tell GL to use the generated mips.
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        EXPECT_EQ(glGetError(), GL_NONE);

        // Now render the textured quad again. It should be still be blue.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

        // Now render the textured quad to an area smaller than the texture (i.e. to force minification). This should be blue.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
        EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);

        // Now clear the texture to green. This just clears the top level. The lower mips should remain blue.
        glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // From now on, default clear color is black.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        if (!bUseMipmapsImmediatelyAfterClearingLevel0)
        {
            // At this point, ANGLE's D3D11 renderer gets into a confused state.
            // The texture is in a good state, with level 0 green and level 1 onwards blue. However, the texture is no longer bound, and ANGLE doesn't realise this.
            // The D3D11 renderer is happy if we disable mips and use the texture, before reenabling mips.

            // Disable mipmaps again
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            EXPECT_EQ(glGetError(), GL_NONE);

            // Render a textured quad equal in size to the texture. This should be green, the color of level 0 in the texture.
            ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
            EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 255, 0, 255);

            // Render a small textured quad. This would force minification if mips were enabled, but they're not. Therefore, this should be green.
            ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
            EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 255, 0, 255);

            // Renable mipmaps again
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
            ASSERT_GL_NO_ERROR();
        }

        // Render a textured quad equal in size to the texture. This should be green, since we just cleared level 0.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 255, 0, 255);

        // Render a small textured quad. This forces minification, so should render blue (the color of levels 1+).
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
        EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);

        // Disable mipmaps again
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        ASSERT_GL_NO_ERROR();

        // Render a textured quad equal in size to the texture. This should be green, the color of level 0 in the texture.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 255, 0, 255);

        // Render a small textured quad. This would force minification if mips were enabled, but they're not. Therefore, this should be green.
        ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
        EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 255, 0, 255);
    }

    GLuint mProgram;
    GLuint mOffscreenFramebuffer;
    GLuint mOffscreenTexture;

    GLint mTextureUniformPosition;
    GLint mPositionAttributePosition;
    GLint mTexCoordAttributePosition;

    GLubyte* mLevelZeroBlueInitData;
    GLubyte* mLevelZeroWhiteInitData;
    GLubyte* mLevelOneInitData;
    GLubyte* mLevelTwoInitData;
};

// This test uses init data for the first three levels of the texture. It passes the level 0 data in, then renders, then level 1, then renders, etc.
// This ensures that renderers using the zero LOD workaround (e.g. D3D11 FL9_3) correctly pass init data to the mipmapped texture, 
// even if the the zero-LOD texture is currently in use.
TYPED_TEST(MipmapTest, ThreelevelsInitData)
{
    // Pass in level zero init data.
    glBindTexture(GL_TEXTURE_2D, mOffscreenTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroBlueInitData);
    ASSERT_GL_NO_ERROR();

    // Disable mips.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Draw a full-sized quad, and check it's blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Draw a half-sized quad, and check it's blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 0, 255, 255);

    // Draw a quarter-sized quad, and check it's blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);

    // Complete the texture by initializing the remaining levels.
    int n = 1;
    while (getWindowWidth() / pow(2, n) >= 1)
    {
        glTexImage2D(GL_TEXTURE_2D, n, GL_RGB, getWindowWidth() / pow(2, n), getWindowWidth() / pow(2, n), 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        ASSERT_GL_NO_ERROR();
        n+=1;
    }

    // Pass in level one init data.
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGB, getWindowWidth() / 2, getWindowHeight() / 2, 0, GL_RGB, GL_UNSIGNED_BYTE, mLevelOneInitData);
    ASSERT_GL_NO_ERROR();

    // Draw a full-sized quad, and check it's blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Draw a half-sized quad, and check it's green.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 255, 0, 255);

    // Draw a quarter-sized quad, and check it's black, since we've not passed any init data for level two.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 0, 255);

    // Pass in level two init data.
    glTexImage2D(GL_TEXTURE_2D, 2, GL_RGB, getWindowWidth() / 4, getWindowHeight() / 4, 0, GL_RGB, GL_UNSIGNED_BYTE, mLevelTwoInitData);
    ASSERT_GL_NO_ERROR();

    // Draw a full-sized quad, and check it's blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Draw a half-sized quad, and check it's green.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 255, 0, 255);

    // Draw a quarter-sized quad, and check it's red.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 255, 0, 0, 255);

    // Now disable mipmaps again, and render multiple sized quads. They should all be blue, since level 0 is blue.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 0, 255, 255);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);

    // Now reset level 0 to white, keeping mipmaps disabled. Then, render various sized quads. They should be white.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroWhiteInitData);
    ASSERT_GL_NO_ERROR();

    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 255, 255, 255);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 255, 255, 255, 255);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 255, 255, 255, 255);

    // Then enable mipmaps again. The quads should be white, green, red respectively.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 255, 255, 255);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 255, 0, 255);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 255, 0, 0, 255);
}

// This test generates (and uses) mipmaps on a texture using init data. D3D11 will use a non-renderable TextureStorage for this.
// The test then disables mips, renders to level zero of the texture, and reenables mips before using the texture again.
// To do this, D3D11 has to convert the TextureStorage into a renderable one.
// This test ensures that the conversion works correctly. 
// In particular, on D3D11 Feature Level 9_3 it ensures that both the zero LOD workaround texture AND the 'normal' texture are copied during conversion.
TYPED_TEST(MipmapTest, GenerateMipmapFromInitDataThenRender)
{
    // Pass in initial data so the texture is blue.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, getWindowWidth(), getWindowHeight(), 0, GL_RGB, GL_UNSIGNED_BYTE, mLevelZeroBlueInitData);

    // Then generate the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mipmaps.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Now draw the texture to various different sized areas.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Use mip level 1
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 0, 255, 255);

    // Use mip level 2
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);

    ASSERT_GL_NO_ERROR();

    // Disable mips. Render a quad using the texture and ensure it's blue.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Clear level 0 of the texture.
    glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Reenable mips, and try rendering different-sized quads.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Level 0 is now red, so this should render red.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);

    // Use mip level 1, blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 0, 255, 255);

    // Use mip level 2, blue.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);

}

// This test ensures that mips are correctly generated from a rendered image.
// In particular, on D3D11 Feature Level 9_3, the clear call will be performed on the zero-level texture, rather than the mipped one.
// The test ensures that the zero-level texture is correctly copied into the mipped texture before the mipmaps are generated.
TYPED_TEST(MipmapTest, GenerateMipmapFromRenderedImage)
{
    // Bind the offscreen framebuffer/texture.
    glBindFramebuffer(GL_FRAMEBUFFER, mOffscreenFramebuffer);

    // Clear the texture to blue.
    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Then generate the mips
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Enable mips.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    // Now draw the texture to various different sized areas.
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth(), getWindowHeight());
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 0, 0, 255, 255);

    // Use mip level 1
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 2, getWindowHeight() / 2);
    EXPECT_PIXEL_EQ(getWindowWidth() / 4, getWindowHeight() / 4, 0, 0, 255, 255);

    // Use mip level 2
    ClearAndDrawTexturedQuad(mOffscreenTexture, getWindowWidth() / 4, getWindowHeight() / 4);
    EXPECT_PIXEL_EQ(getWindowWidth() / 8, getWindowHeight() / 8, 0, 0, 255, 255);
}

// Test to ensure that rendering to a mipmapped texture works, regardless of whether mipmaps are enabled or not.
// TODO: This test hits a texture rebind bug in the D3D11 renderer. Fix this.
TYPED_TEST(MipmapTest, RenderOntoLevelZeroAfterGenerateMipmap)
{
    RenderAfterGenerateMipmapBase(true);
}

// This is very similar to RenderOntoLevelZeroAfterGenerateMipmap, but it works around the texture rebind bug in the D3D11 renderer.
// It ensures the mipmaps work as expected in this scenario.
TYPED_TEST(MipmapTest, RenderOntoLevelZeroAfterGenerateMipmapWithMipmapStateChange)
{   
    RenderAfterGenerateMipmapBase(false);
}