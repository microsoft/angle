//            Based on Hello_Triangle.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

//
// This file is used by the template to render a basic scene using GL.
//

#include "pch.h"
#include "HelloTriangleRenderer.h"

// These are used by the shader compilation methods.
#include <vector>
#include <iostream>
#include <fstream>

using namespace $safeprojectname$;

#define STRING(s) #s

GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLog.size(), NULL, infoLog.data());

        std::wstring errorMessage = std::wstring(L"Shader compilation failed: ");
        errorMessage += std::wstring(infoLog.begin(), infoLog.end());
        OutputDebugStringW(errorMessage.c_str());

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

GLuint CompileProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = glCreateProgram();

    if (program == 0)
    {
        OutputDebugStringW(L"Program creation failed");
        return 0;
    }

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLog.size(), NULL, infoLog.data());

        std::wstring errorMessage = std::wstring(L"Shader linking failed: ");
        errorMessage += std::wstring(infoLog.begin(), infoLog.end());
        OutputDebugStringW(errorMessage.c_str());

        glDeleteProgram(program);
        return 0;
    }

    return program;
}

HelloTriangleRenderer::HelloTriangleRenderer() :
    mProgram(0)
{
}

bool HelloTriangleRenderer::Initialize() 
{
    const std::string vs = STRING
    (
        attribute vec4 vPosition;
        void main()
        {
            gl_Position = vPosition;
        }
    );

    const std::string fs = STRING
    (
        precision mediump float;
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    mProgram = CompileProgram(vs, fs);
    if (!mProgram)
    {
        return false;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return true;
}

HelloTriangleRenderer::~HelloTriangleRenderer()
{
    if (mProgram != 0)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }
}

// Draws a basic triangle
void HelloTriangleRenderer::Draw()
{
    GLfloat vertices[] =
    {
         0.0f,  0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
    };

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(mProgram);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}

void HelloTriangleRenderer::OnWindowSizeChanged(GLsizei width, GLsizei height)
{
    glViewport(0, 0, width, height);
}
