#include "ANGLETest.h"

#include <vector>

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_TYPED_TEST_CASE(FeatureLevel9Test, ES2_D3D11_FL9_3);

template<typename T>
class FeatureLevel9Test : public ANGLETest
{
protected:
    FeatureLevel9Test() : ANGLETest(T::GetGlesMajorVersion(), T::GetPlatform())
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
		// Point Sprite Compilation Test Vertex Shader source
		pointSpriteCompilationTestVS = SHADER_SOURCE
		(
			void main()
			{
				gl_PointSize = 100.0;
				gl_Position = vec4(1.0, 0.0, 0.0, 1.0);
			}
		);

		// Point Sprite Compilation Test Fragment Shader source
		pointSpriteCompilationTestFS = SHADER_SOURCE
		(
			precision mediump float;
			void main()
			{
				gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
			}
		);

        ANGLETest::SetUp();
    }

	std::string pointSpriteCompilationTestVS;
	std::string pointSpriteCompilationTestFS;

    // Helper that checks for D3D_FEATURE_LEVEL_9_* by looking for the a hint in the renderer string.
    bool findFL9RendererString()
    {
        std::string rendererString = std::string((char*)glGetString(GL_RENDERER));
        std::transform(rendererString.begin(), rendererString.end(), rendererString.begin(), ::tolower);
        return (rendererString.find(std::string("level_9")) != std::string::npos);
    }
};

TYPED_TEST(FeatureLevel9Test, pointSpriteCompile)
{
    bool featureLevel9StringFound = findFL9RendererString();
    EXPECT_EQ(true, featureLevel9StringFound);

    // Compiling the shaders should succeed. It's linking them that should fail.
    GLuint program = glCreateProgram();
    ASSERT_GL_NO_ERROR();
    ASSERT_NE(program, 0U);

    GLuint vs = compileShader(GL_VERTEX_SHADER, pointSpriteCompilationTestVS);
    ASSERT_GL_NO_ERROR();
    ASSERT_NE(vs, 0U);

    GLuint fs = compileShader(GL_FRAGMENT_SHADER, pointSpriteCompilationTestFS);
    ASSERT_GL_NO_ERROR();
    ASSERT_NE(fs, 0U);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
    }

    glAttachShader(program, vs);
    ASSERT_GL_NO_ERROR();
    glDeleteShader(vs);

    glAttachShader(program, fs);
    ASSERT_GL_NO_ERROR();
    glDeleteShader(fs);

    // Linking should fail on D3D_FEATURE_LEVEL_9_*.
    GLint linkStatus;
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    ASSERT_EQ(0, linkStatus);

    // If the compilation failed, then check that it failed for the expected reason.
    // Do this by checking that the error log mentions Geometry Shaders.
    GLint infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    std::vector<GLchar> infoLog(infoLogLength);
    glGetProgramInfoLog(program, (GLsizei)infoLog.size(), NULL, infoLog.data());

    std::string logString = std::string(infoLog.begin(), infoLog.end());
    std::transform(logString.begin(), logString.end(), logString.begin(), ::tolower);
    bool geometryShaderFoundInLog = (logString.find(std::string("geometry shader")) != std::string::npos);
    ASSERT_EQ(true, geometryShaderFoundInLog);

    glDeleteProgram(program);
}