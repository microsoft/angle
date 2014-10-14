//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// winrtutils.cpp: Common Windows Runtime utilities.

#include <wrl.h>
#include <wrl/wrappers/corewrappers.h>
#include <windows.graphics.display.h>
#include "common/winrt/winrtutils.h"
#include "common/winrt/CoreWindowNativeWindow.h"
#include "common/debug.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::ApplicationModel::Core;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;

namespace winrt
{

static float GetLogicalDpi()
{
    ComPtr<ABI::Windows::Graphics::Display::IDisplayPropertiesStatics> displayProperties;
    float dpi = 96.0f;

    if (SUCCEEDED(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayProperties).Get(), displayProperties.GetAddressOf())))
    {
        if (SUCCEEDED(displayProperties->get_LogicalDpi(&dpi)))
        {
            return dpi;
        }
    }
    return dpi;
}

long convertDipsToPixels(float dips)
{
    static const float dipsPerInch = 96.0f;
    return lround((dips * GetLogicalDpi() / dipsPerInch));
}

};