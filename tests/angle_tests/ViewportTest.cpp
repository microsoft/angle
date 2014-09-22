#include "ANGLETest.h"

class ViewportTest : public ANGLETest
{
protected:
    ViewportTest()
    {
        setWindowWidth(512);
        setWindowHeight(512);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
    }

    void runTest()
    {
        // Firstly ensure that no errors have been hit.
        EXPECT_GL_NO_ERROR();

        GLint viewportSize[4];
        glGetIntegerv(GL_VIEWPORT, viewportSize);

        // Draw a red quad centered in the middle of the viewport, with dimensions 25% of the size of the viewport.
        drawQuad(mProgram, "position", 0.5f, 0.25f);

        GLint centerViewportX = viewportSize[0] + (viewportSize[2] / 2);
        GLint centerViewportY = viewportSize[1] + (viewportSize[3] / 2);
        
        GLint redQuadLeftSideX   = viewportSize[0] + viewportSize[2] * 3 / 8;
        GLint redQuadRightSideX  = viewportSize[0] + viewportSize[2] * 5 / 8;
        GLint redQuadTopSideY    = viewportSize[1] + viewportSize[3] * 3 / 8;
        GLint redQuadBottomSideY = viewportSize[1] + viewportSize[3] * 5 / 8;

        // The midpoint of the viewport should be red.
        checkPixel(centerViewportX, centerViewportY, true);

        // Pixels just inside the red quad should be red.
        // Use +/- 2 to ensure that the test isn't impacted by division rounding (neither in the test nor in the viewport emulation shader code).
        checkPixel(redQuadLeftSideX + 2,  redQuadTopSideY + 2,    true);
        checkPixel(redQuadLeftSideX + 2,  redQuadBottomSideY - 2, true);
        checkPixel(redQuadRightSideX - 2, redQuadTopSideY + 2,    true);
        checkPixel(redQuadRightSideX - 2, redQuadBottomSideY - 2, true);

        // Pixels just outside the red quad should be black.
        checkPixel(redQuadLeftSideX - 2,  redQuadTopSideY - 2,    false);
        checkPixel(redQuadLeftSideX - 2,  redQuadBottomSideY + 2, false);
        checkPixel(redQuadRightSideX + 2, redQuadTopSideY - 2,    false);
        checkPixel(redQuadRightSideX + 2, redQuadBottomSideY + 2, false);

        // Pixels just within the viewport should be black.
        checkPixel(viewportSize[0] + 2,                    viewportSize[1] + 2,                   false);
        checkPixel(viewportSize[0] + 2,                    viewportSize[1] + viewportSize[3] - 2, false);
        checkPixel(viewportSize[0] + viewportSize[2] - 2,  viewportSize[1] + 2,                   false);
        checkPixel(viewportSize[0] + viewportSize[2] - 2,  viewportSize[1] + viewportSize[3] - 2, false);
    }

    void checkPixel(GLint x, GLint y, GLboolean expectRed)
    {
        GLint expectedRedChannel = expectRed ? 255 : 0;

        // If the pixel is within the bounds of the window, then we check it. Otherwise we skip it.
        if (0 <= x && x < getWindowWidth() && 0 <= y && y < getWindowHeight())
        {
            EXPECT_PIXEL_EQ(x, y, expectedRedChannel, 0, 0, 255);
        }
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string testVertexShaderSource = SHADER_SOURCE
        (
            attribute highp vec4 position;

            void main(void)
            {
                gl_Position = position;
            }
        );

        const std::string testFragmentShaderSource = SHADER_SOURCE
        (
            void main(void)
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        );

        mProgram = CompileProgram(testVertexShaderSource, testFragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 255);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Call glViewport with default parameters.
        glViewport(0, 0, getWindowWidth(), getWindowHeight());

        glDisable(GL_DEPTH_TEST);
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_F(ViewportTest, QuarterWindow)
{
    glViewport(0, 0, getWindowWidth() / 4, getWindowHeight() / 4);
    runTest();
}

TEST_F(ViewportTest, QuarterWindowCentered)
{
    glViewport(getWindowWidth() * 3 / 8, getWindowHeight() * 3 / 8, getWindowWidth() / 4, getWindowHeight() / 4);
    runTest();
}

TEST_F(ViewportTest, FullWindow)
{
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    runTest();
}

TEST_F(ViewportTest, FullWindowOffCenter)
{
    glViewport(-getWindowWidth() / 2, getWindowHeight() / 2, getWindowWidth(), getWindowHeight());
    runTest();
}

TEST_F(ViewportTest, DoubleWindow)
{
    glViewport(0, 0, getWindowWidth() * 2, getWindowHeight() * 2);
    runTest();
}

TEST_F(ViewportTest, DoubleWindowCentered)
{
    glViewport(-getWindowWidth() / 2, -getWindowHeight() / 2, getWindowWidth() * 2, getWindowHeight() * 2);
    runTest();
}

TEST_F(ViewportTest, DoubleWindowOffCenter)
{
    glViewport(-getWindowWidth() * 3 / 4, getWindowHeight() * 3 / 4, getWindowWidth(), getWindowHeight());
    runTest();
}

TEST_F(ViewportTest, TripleWindow)
{
    glViewport(0, 0, getWindowWidth() * 3, getWindowHeight() * 3);
    runTest();
}

TEST_F(ViewportTest, TripleWindowCentered)
{
    glViewport(-getWindowWidth(), -getWindowHeight(), getWindowWidth() * 3, getWindowHeight() * 3);
    runTest();
}

TEST_F(ViewportTest, TripleWindowOffCenter)
{
    glViewport(-getWindowWidth() * 3 / 2, -getWindowHeight() * 3 / 2, getWindowWidth() * 3, getWindowHeight() * 3);
    runTest();
}