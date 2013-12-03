#pragma once

#include "Direct3DBase.h"
#include <DirectXMath.h>
#include "BasicTimer.h"

// This class renders a simple spinning cube.
ref class CubeRenderer sealed : public Direct3DBase
{
public:
	CubeRenderer();

	// Direct3DBase methods.
	virtual void CreateDeviceResources() override;
	virtual void CreateWindowSizeDependentResources() override;
	virtual void Render() override;
	void Update(float timeTotal, float timeDelta);
	virtual void OnOrientationChanged() override;

private:
    BasicTimer^ m_timer;
    GLuint m_colorProgram;
    GLint a_positionColor;
    GLint a_colorColor;
    GLint u_mvpColor;
    DirectX::XMMATRIX m_projectionMatrix;
    DirectX::XMMATRIX m_viewMatrix;
    DirectX::XMMATRIX m_modelMatrix;
};
