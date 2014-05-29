//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

#include "pch.h"

#define SHADER_SOURCE(...) #__VA_ARGS__

inline GLuint CompileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        return 0;
    }

    return shader;
}

inline GLuint CompileShadersIntoProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = glCreateProgram();

    if (program == 0)
    {
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

    return program;
}

inline GLuint CompileAndLinkProgram(const std::string &vsSource, const std::string &fsSource)
{
    GLuint program = CompileShadersIntoProgram(vsSource, fsSource);

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        return 0;
    }

    return program;
}