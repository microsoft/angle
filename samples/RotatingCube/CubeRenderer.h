#pragma once

#include "esUtil.h"
#include <DirectXMath.h>

// This class renders a simple spinning cube.
class CubeRenderer
{
public:
	//CubeRenderer();

    void CreateResources();
    void UpdateForWindowSizeChanged();
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_PHONE)
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
    DirectX::XMMATRIX m_projectionMatrix;
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_modelMatrix;
};
