#pragma once

#include "pch.h"
#include "CubeRenderer.h"
#include "esUtil.h"
#include <GLES2/gl2ext.h>
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

ref class Asteroids sealed : public Windows::ApplicationModel::Core::IFrameworkView
{
public:
	Asteroids();
	
	// IFrameworkView Methods.
	virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
	virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
	virtual void Load(Platform::String^ entryPoint);
	virtual void Run();
	virtual void Uninitialize();

protected:
	// Event Handlers.
	void OnWindowSizeChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::WindowSizeChangedEventArgs^ args);
	void OnLogicalDpiChanged(Platform::Object^ sender);
	void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
	void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ args);
	void OnResuming(Platform::Object^ sender, Platform::Object^ args);
	void OnWindowClosed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::CoreWindowEventArgs^ args);
	void OnVisibilityChanged(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::VisibilityChangedEventArgs^ args);
	void OnPointerPressed(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
	void OnPointerMoved(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::PointerEventArgs^ args);
    void OnKeyDown(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);
    void OnKeyUp(Windows::UI::Core::CoreWindow^ sender, Windows::UI::Core::KeyEventArgs^ args);

private:
    struct GameObject
    {
        GameObject(void);
        void Update(float dt);
        glm::mat4 TransformMatrix(void) const;

        float m_angle;
        float m_omega;
        glm::vec2 m_acc;
        glm::vec2 m_vel;
        glm::vec2 m_pos;
        glm::vec2 m_scale;
        int m_lives;
    };

    typedef std::vector<GameObject> GameObjectList;
    typedef GameObjectList::iterator GameObjectIter;
    
    void Update();
    void Draw();
    void WrapAround(GameObject &object);
    void FireBullet();
    void RemoveOutOfBoundsBullets();
    void DrawPlayer(void);
    void DrawAsteroids(void);
    void DrawBullets(void);

    ESContext m_esContext;
    GLuint m_program;
    GLint m_mvp;
    GLint m_aPosition;
    GLint m_aColor;
    GLint m_colorLoc;
    GameObject m_player;
    GameObjectList m_asteroids;
    GameObjectList m_bullets;
	//CubeRenderer^ m_renderer;
    std::vector<float> m_vertexBuffer;
    unsigned m_playerState;
    float m_bulletTimer;
	bool m_windowClosed;
	bool m_windowVisible;
};

ref class Direct3DApplicationSource sealed : Windows::ApplicationModel::Core::IFrameworkViewSource
{
public:
	virtual Windows::ApplicationModel::Core::IFrameworkView^ CreateView();
};
