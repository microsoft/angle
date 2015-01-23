#pragma once

#include "pch.h"

namespace $ext_safeprojectname$
{
    class HelloTriangleRenderer
    {
    public:
        HelloTriangleRenderer();
        ~HelloTriangleRenderer();
        void Draw();
        void UpdateWindowSize(GLsizei width, GLsizei height);

    private:
        GLuint mProgram;
    };
}