#include "pch.h"
#include "Asteroids.h"
#include "BasicTimer.h"

#define STRINGIFY(str) #str
#define PLAYER_ACCELERATION 100.0f
#define TURN_SPEED 100.0f
#define MAX_ASTEROIDS 32
#define BULLET_SPEED 300.0f
#define DESTROYED_ASTEROID_SPEED 100.0f

#define PLAYER_STATE_W     0x1
#define PLAYER_STATE_A     0x2
#define PLAYER_STATE_S     0x4
#define PLAYER_STATE_D     0x8
#define PLAYER_STATE_SPACE 0x10

using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;
using namespace concurrency;
using namespace DirectX;

Asteroids::Asteroids() :
    m_playerState(0),
    m_bulletTimer(0),
	m_windowClosed(false),
	m_windowVisible(true)
{
}

void Asteroids::Initialize(CoreApplicationView^ applicationView)
{
	applicationView->Activated +=
        ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &Asteroids::OnActivated);

	CoreApplication::Suspending +=
        ref new EventHandler<SuspendingEventArgs^>(this, &Asteroids::OnSuspending);

	CoreApplication::Resuming +=
        ref new EventHandler<Platform::Object^>(this, &Asteroids::OnResuming);

	//m_renderer = ref new CubeRenderer();
    m_player.m_scale = glm::vec2(40);
    for(int i = 0; i < MAX_ASTEROIDS; ++i)
    {
        GameObject asteroid;
        asteroid.m_pos.x = static_cast<float>(rand() % 10000);
        asteroid.m_pos.y = static_cast<float>(rand() % 10000);
        asteroid.m_angle = (rand() % 1000) / 10.0f - 50;
        asteroid.m_omega = (rand() % 1000) / 10.0f - 50;
        asteroid.m_vel.x = (rand() % 1000) / 10.0f - 50;
        asteroid.m_vel.y = (rand() % 1000) / 10.0f - 50;
        asteroid.m_scale.x = (rand() % 1000) / 20.0f + 50;
        asteroid.m_scale.y = asteroid.m_scale.x;//(rand() % 1000) / 10.0f - 50;
        asteroid.m_lives = 2;
        m_asteroids.push_back(asteroid);
    }
}

void Asteroids::SetWindow(CoreWindow^ window)
{
	window->SizeChanged += 
        ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &Asteroids::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &Asteroids::OnVisibilityChanged);

	window->Closed += 
        ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &Asteroids::OnWindowClosed);

	window->PointerCursor = ref new CoreCursor(CoreCursorType::Arrow, 0);

	window->PointerPressed +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Asteroids::OnPointerPressed);

	window->PointerMoved +=
		ref new TypedEventHandler<CoreWindow^, PointerEventArgs^>(this, &Asteroids::OnPointerMoved);

    window->KeyDown +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &Asteroids::OnKeyDown);

    window->KeyUp +=
        ref new TypedEventHandler<CoreWindow^, KeyEventArgs^>(this, &Asteroids::OnKeyUp);

	//m_renderer->Initialize(CoreWindow::GetForCurrentThread());
    
    esInitContext ( &m_esContext );
    m_esContext.hWnd = CoreWindow::GetForCurrentThread();
    esCreateWindow ( &m_esContext, TEXT("Simple Instancing"), 320, 240, ES_WINDOW_RGB );

    const char *vs = STRINGIFY(
    attribute vec3 a_position;
    attribute vec4 a_color;
    varying vec4 v_color;
    uniform mat4 u_mvp;
    void main(void)
    {
        v_color = a_color;
        gl_Position = u_mvp * vec4(a_position, 1);
    }
    );

    const char *fs = STRINGIFY(
    precision mediump float;
    varying vec4 v_color;
    void main(void)
    {
        gl_FragColor = v_color;
    }
    );

    m_program = esLoadProgram(vs, fs);
    m_mvp = glGetUniformLocation(m_program, "u_mvp");
    m_aPosition = glGetAttribLocation(m_program, "a_position");
    m_aColor = glGetAttribLocation(m_program, "a_color");
}

void Asteroids::Load(Platform::String^ entryPoint)
{
}

void Asteroids::Run()
{
	BasicTimer^ timer = ref new BasicTimer();
	while (!m_windowClosed)
	{
		if (m_windowVisible)
		{
			timer->Update();
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessAllIfPresent);
            Update();
            Draw();


            
			//m_renderer->Update(timer->Total, timer->Delta);
			//m_renderer->Render();
			//m_renderer->Present(); // This call is synchronized to the display frame rate.
		}
		else
		{
			CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
		}
	}
}

void Asteroids::Uninitialize()
{
}

void Asteroids::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	//m_renderer->UpdateForWindowSizeChange();
}

void Asteroids::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	m_windowVisible = args->Visible;
}

void Asteroids::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

void Asteroids::OnPointerPressed(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void Asteroids::OnPointerMoved(CoreWindow^ sender, PointerEventArgs^ args)
{
	// Insert your code here.
}

void Asteroids::OnKeyDown(CoreWindow^ sender, KeyEventArgs ^args)
{
    switch(args->VirtualKey)
    {
        case VirtualKey::W:
            m_playerState |= PLAYER_STATE_W;
            break;
        
        case VirtualKey::S:
            m_playerState |= PLAYER_STATE_S;
            break;

        case VirtualKey::A:
            m_playerState |= PLAYER_STATE_A;
            break;

        case VirtualKey::D:
            m_playerState |= PLAYER_STATE_D;
            break;

        case VirtualKey::Space:
            m_playerState |= PLAYER_STATE_SPACE;
            break;
    }
}

void Asteroids::OnKeyUp(CoreWindow^ sender, KeyEventArgs ^args)
{
    switch(args->VirtualKey)
    {
        case VirtualKey::W:
            m_playerState &= ~PLAYER_STATE_W;
            break;
        
        case VirtualKey::S:
            m_playerState &= ~PLAYER_STATE_S;
            break;

        case VirtualKey::A:
            m_playerState &= ~PLAYER_STATE_A;
            break;

        case VirtualKey::D:
            m_playerState &= ~PLAYER_STATE_D;
            break;
            
        case VirtualKey::Space:
            m_playerState &= ~PLAYER_STATE_SPACE;
            break;
    }
}

void Asteroids::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	CoreWindow::GetForCurrentThread()->Activate();
}

void Asteroids::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	create_task([this, deferral]()
	{
		// Insert your code here.

		deferral->Complete();
	});
}
 
void Asteroids::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.
}

Asteroids::GameObject::GameObject(void) : m_angle(0), m_omega(0), m_scale(1, 1), m_lives(1)
{
}

void Asteroids::GameObject::Update(float dt)
{
    //do semi-implicit euler
    m_vel += m_acc * dt;
    m_pos += m_vel * dt;
    m_angle += m_omega * dt;
}

glm::mat4 Asteroids::GameObject::TransformMatrix(void) const
{
    return glm::translate(glm::vec3(m_pos, 0)) * glm::rotate(m_angle, glm::vec3(0, 0, 1)) * glm::scale(glm::vec3(m_scale, 0));
}

static float MagnitudeSquared(float x, float y)
{
    return x * x + y * y;
}

static float MagnitudeSquared(const glm::vec2 &v)
{
    return MagnitudeSquared(v.x, v.y);
}

void Asteroids::Update()
{
    const float dt = 1.0f / 60;

    //handle input
    float radians = glm::radians(m_player.m_angle);
    if((m_playerState & PLAYER_STATE_W) && !(m_playerState & PLAYER_STATE_S))
        m_player.m_acc = glm::vec2(cos(radians), sin(radians)) * PLAYER_ACCELERATION;
    if((m_playerState & PLAYER_STATE_S) && !(m_playerState &PLAYER_STATE_W))
        m_player.m_acc = glm::vec2(cos(radians), sin(radians)) * -PLAYER_ACCELERATION;
    else if(!(m_playerState & PLAYER_STATE_S) && !(m_playerState &PLAYER_STATE_W))
        m_player.m_acc = glm::vec2();
    if(m_playerState & PLAYER_STATE_A)
        m_player.m_omega = TURN_SPEED;
    if(m_playerState & PLAYER_STATE_D)
        m_player.m_omega = -TURN_SPEED;
    if(!(m_playerState & PLAYER_STATE_A) && !(m_playerState & PLAYER_STATE_D))
        m_player.m_omega = 0;
    m_bulletTimer -= dt;
    if(m_playerState & PLAYER_STATE_SPACE)
        FireBullet();

    //move everything around
    m_player.Update(dt);
    GameObjectIter end = m_asteroids.end();
    for(GameObjectIter it = m_asteroids.begin(); it != end; ++it)
        it->Update(dt);
    end = m_bullets.end();
    for(GameObjectIter it = m_bullets.begin(); it != end; ++it)
        it->Update(dt);

    //wrap player and asteroids to the other side of the universe if out of bounds
    WrapAround(m_player);
    end = m_asteroids.end();
    for(GameObjectIter it = m_asteroids.begin(); it != end; ++it)
        WrapAround(*it);

    //collide the asteroids and bullets using circles
    end = m_bullets.end();
    for(GameObjectIter it = m_bullets.begin(); it != end; )
    {
        GameObjectIter end2 = m_asteroids.end();
        for(GameObjectIter it2 = m_asteroids.begin(); it2 != end2;)
        {
            float radius1 = MagnitudeSquared(it->m_scale * 0.5f);
            float radius2 = MagnitudeSquared(it2->m_scale * 0.5f);
            float distance = MagnitudeSquared(it->m_pos - it2->m_pos);

            //they've collided so delete them
            if(radius1 + radius2 > distance)
            {
                it = m_bullets.erase(it);
                end = m_bullets.end();

                //split the asteroid into 4 smaller ones
                if(it2->m_lives == 2)
                {
                    int position = it2 - m_asteroids.begin();
                    GameObject asteroid;
                    float radians = glm::radians(glm::radians(it2->m_angle + 45));
                    glm::vec2 vel = it2->m_vel;
                    glm::vec2 pos = it2->m_pos;
                    glm::vec2 dir1(cos(radians), sin(radians));
                    glm::vec2 dir2(dir1.y, -dir1.x);
                    float scale = it2->m_scale.x;
                    float angle = it2->m_angle;
                    float omega = it2->m_omega;

                    asteroid.m_scale.x = asteroid.m_scale.y = scale * 0.5f;
                    asteroid.m_pos = pos + dir1 * scale * 0.25f;
                    asteroid.m_angle = angle;
                    asteroid.m_omega = omega + (rand() % 1000) / 10.0f - 50;
                    asteroid.m_vel = vel + dir1 * DESTROYED_ASTEROID_SPEED;
                    m_asteroids.push_back(asteroid);

                    asteroid.m_pos = pos - dir1 * scale * 0.25f;
                    asteroid.m_angle = angle;
                    asteroid.m_omega = omega + (rand() % 1000) / 10.0f - 50;
                    asteroid.m_vel = vel - dir1 * DESTROYED_ASTEROID_SPEED;
                    m_asteroids.push_back(asteroid);

                    asteroid.m_pos = pos + dir2 * scale * 0.25f;
                    asteroid.m_angle = angle;
                    asteroid.m_omega = omega + (rand() % 1000) / 10.0f - 50;
                    asteroid.m_vel = vel + dir2 * DESTROYED_ASTEROID_SPEED;
                    m_asteroids.push_back(asteroid);

                    asteroid.m_pos = pos - dir2 * scale * 0.25f;
                    asteroid.m_angle = angle;
                    asteroid.m_omega = omega + (rand() % 1000) / 10.0f - 50;
                    asteroid.m_vel = vel - dir2 * DESTROYED_ASTEROID_SPEED;
                    m_asteroids.push_back(asteroid);

                    it2 = m_asteroids.begin() + position;
                }
                it2 = m_asteroids.erase(it2);
                end2 = m_asteroids.end();
                if(it == end)
                    break;
            }
            else
                ++it2;
        }
        if(it != end)
            ++it;
    }
}

void Asteroids::Draw()
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program);

    CoreWindow ^window = CoreWindow::GetForCurrentThread();
    float halfWindowWidth = window->Bounds.Width * 0.5f;
    float halfWindowHeight = window->Bounds.Height * 0.5f;
    glm::mat4 ortho = glm::ortho(-halfWindowWidth, halfWindowWidth, -halfWindowHeight, halfWindowHeight, -1.0f, 1.0f);

    glUniformMatrix4fv(m_mvp, 1, GL_FALSE, &ortho[0][0]);

    DrawPlayer();
    DrawAsteroids();
    DrawBullets();
    
    const int stride = sizeof(glm::vec3) + sizeof(glm::vec4);
    glVertexAttribPointer(m_aPosition, 3, GL_FLOAT, GL_FALSE, stride, &m_vertexBuffer[0]);
    glEnableVertexAttribArray(m_aPosition);
    glVertexAttribPointer(m_aColor, 4, GL_FLOAT, GL_FALSE, stride, &m_vertexBuffer[3]);
    glEnableVertexAttribArray(m_aColor);

    glDrawArrays(GL_LINES, 0, m_vertexBuffer.size() / 7);
    m_vertexBuffer.clear();
    eglSwapBuffers(m_esContext.eglDisplay, m_esContext.eglSurface);  
}

void Asteroids::WrapAround(GameObject &object)
{
    CoreWindow ^window = CoreWindow::GetForCurrentThread();
    float halfWindowWidth = window->Bounds.Width * 0.5f;
    float halfWindowHeight = window->Bounds.Height * 0.5f;

    if(object.m_pos.x - object.m_scale.x > halfWindowWidth)
        object.m_pos.x = -halfWindowWidth - object.m_scale.x * 0.5f;
    if(object.m_pos.x + object.m_scale.x < -halfWindowWidth)
        object.m_pos.x = halfWindowWidth + object.m_scale.x * 0.5f;

    if(object.m_pos.y - object.m_scale.y > halfWindowHeight)
        object.m_pos.y = -halfWindowHeight - object.m_scale.y * 0.5f;
    if(object.m_pos.y + object.m_scale.y < -halfWindowHeight)
        object.m_pos.y = halfWindowHeight + object.m_scale.y * 0.5f;
}

void Asteroids::FireBullet()
{
    if(m_bulletTimer <= 0)
    {
        float radians = glm::radians(m_player.m_angle);
        GameObject bullet;
        glm::vec2 dir(cos(radians), sin(radians));
        bullet.m_pos = m_player.m_pos + dir * m_player.m_scale.x * 0.5f;
        bullet.m_scale = glm::vec2(10, 10);
        bullet.m_angle = m_player.m_angle;
        bullet.m_vel = dir * BULLET_SPEED + m_player.m_vel;
        m_bullets.push_back(bullet);
        m_bulletTimer = 0.2f;
    }
}

void Asteroids::RemoveOutOfBoundsBullets()
{
    CoreWindow ^window = CoreWindow::GetForCurrentThread();
    float halfWindowWidth = window->Bounds.Width * 0.5f;
    float halfWindowHeight = window->Bounds.Height * 0.5f;

    GameObjectIter end = m_bullets.end();
    for(GameObjectIter it = m_bullets.begin(); it != end;)
    {
        if(it->m_pos.x - it->m_scale.x > halfWindowWidth || it->m_pos.x + it->m_scale.x < halfWindowWidth ||
           it->m_pos.y - it->m_scale.y > halfWindowHeight || it->m_pos.y + it->m_scale.y < halfWindowHeight)
            it = m_bullets.erase(it);
        else
            ++it;
    }
}

static void FillVertexBuffer(std::vector<float> &buffer, const glm::vec3 &position, const glm::vec4 &color)
{
    buffer.insert(buffer.end(), &position.x, &position.x + 3);
    buffer.insert(buffer.end(), &color.x, &color.x + 4);
}

void Asteroids::DrawPlayer(void)
{
    glm::mat4 transform = m_player.TransformMatrix();
    glm::vec4 verts[] = {
        glm::vec4(0.5f, 0, 0, 1),
        glm::vec4(-0.5f, 0.3f, 0, 1),
        glm::vec4(-0.5f, 0.3f, 0, 1),
        glm::vec4(-0.2f, 0, 0, 1),
        glm::vec4(-0.2f, 0, 0, 1),
        glm::vec4(-0.5f, -0.3f, 0, 1),
        glm::vec4(-0.5f, -0.3f, 0, 1),
        glm::vec4(0.5f, 0, 0, 1),
    };
    for(int i = 0; i < sizeof(verts) / sizeof(*verts); ++i)
        FillVertexBuffer(m_vertexBuffer, glm::vec3(transform * verts[i]), glm::vec4(0, 1, 0, 1));
}

void Asteroids::DrawAsteroids(void)
{
    glm::vec4 verts[] = {
        glm::vec4(0.5f, 0.5f, 0, 1),
        glm::vec4(-0.5f, 0.5f, 0, 1),
        glm::vec4(-0.5f, 0.5f, 0, 1),
        glm::vec4(-0.5f, -0.5f, 0, 1),
        glm::vec4(-0.5f, -0.5f, 0, 1),
        glm::vec4(0.5f, -0.5f, 0, 1),
        glm::vec4(0.5f, -0.5f, 0, 1),
        glm::vec4(0.5f, 0.5f, 0, 1),
    };
    GameObjectIter end = m_asteroids.end();
    for(GameObjectIter it = m_asteroids.begin(); it != end; ++it)
    {
        glm::mat4 transform = it->TransformMatrix();
        for(int i = 0; i < sizeof(verts) / sizeof(*verts); ++i)
            FillVertexBuffer(m_vertexBuffer, glm::vec3(transform * verts[i]), glm::vec4(1, 0, 0, 1));
    }
}

void Asteroids::DrawBullets(void)
{
    glm::vec4 verts[] = {
        glm::vec4(0.5f, 0, 0, 1),
        glm::vec4(-0.5f, 0.5f, 0, 1),
        glm::vec4(-0.5f, 0.5f, 0, 1),
        glm::vec4(-0.5f, -0.5f, 0, 1),
        glm::vec4(-0.5f, -0.5f, 0, 1),
        glm::vec4(0.5f, 0, 0, 1),
    };
    GameObjectIter end = m_bullets.end();
    for(GameObjectIter it = m_bullets.begin(); it != end; ++it)
    {
        glm::mat4 transform = it->TransformMatrix();
        for(int i = 0; i < sizeof(verts) / sizeof(*verts); ++i)
            FillVertexBuffer(m_vertexBuffer, glm::vec3(transform * verts[i]), glm::vec4(0, 0.5f, 1, 1));
    }
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
    return ref new Asteroids();
}

[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}
