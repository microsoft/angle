#pragma once

#include "app.g.h"
#include "OpenGLES.h"
#include "openglespage.xaml.h"

namespace $ext_safeprojectname$
{
    ref class App sealed
    {
    public:
        App();
        virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

    private:
        OpenGLESPage^ mPage;
        OpenGLES mOpenGLES;
    };
}
