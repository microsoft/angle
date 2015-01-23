#pragma once

#include "pch.h"

namespace $safeprojectname$
{
    class HelloTriangleRenderer
    {
    public:
        HelloTriangleRenderer();
        ~HelloTriangleRenderer();
        void Draw();
        void OnWindowSizeChanged(GLsizei width, GLsizei height);
        bool Initialize();

    private:
        GLuint mProgram;
    };
}