#pragma once

#include "esUtil.h"
#include <GLES2/gl2ext.h>
#include <DirectXMath.h>

using namespace DirectX;

// This class renders a simple spinning cube.
class CubeRenderer
{
public:
	//CubeRenderer();

    void CreateResources();
    void UpdateForWindowSizeChanged();
#if defined(ANGLE_PLATFORM_WP8)
	void OnOrientationChanged();
#endif

	void Render();
	
	// Method for updating time-dependent objects.
	void Update(float timeTotal, float timeDelta);

private:
	Windows::Foundation::Rect m_windowBounds;
	Windows::Graphics::Display::DisplayOrientations m_orientation;
    GLuint m_colorProgram;
    GLint a_positionColor;
    GLint a_colorColor;
    GLint u_mvpColor;
    XMMATRIX m_projectionMatrix;
    XMMATRIX m_viewMatrix;
    XMMATRIX m_modelMatrix;
};
