#include "pch.h"
#include "App.xaml.h"

using namespace $safeprojectname$;

App::App()
{
    InitializeComponent();
}

void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
#if _DEBUG
    if (IsDebuggerPresent())
    {
        DebugSettings->EnableFrameRateCounter = true;
    }
#endif

    if (mPage == nullptr)
    {
        mPage = ref new OpenGLESPage(&mOpenGLES);
    }

    // Place the page in the current window and ensure that it is active.
    Windows::UI::Xaml::Window::Current->Content = mPage;
    Windows::UI::Xaml::Window::Current->Activate();
}
