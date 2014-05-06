//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// winrtutils.cpp: Common Windows Runtime utilities.

#include <wrl.h>
#include <windows.graphics.display.h>
#include "common/winrt/winrtutils.h"
#include "common/winrt/corewindowhost.h"
#include "common/debug.h"

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;

namespace winrt
{

std::string getTempPath()
{
    ComPtr<IActivationFactory> pActivationFactory;
    ComPtr<ABI::Windows::ApplicationModel::IPackageStatics> packageStatics;
    ComPtr<ABI::Windows::ApplicationModel::IPackage> package;
    ComPtr<ABI::Windows::Storage::IStorageFolder> storageFolder;
    ComPtr<ABI::Windows::Storage::IStorageItem> storageItem;
    HString hstrPath;

    HRESULT result = GetActivationFactory(HStringReference(RuntimeClass_Windows_ApplicationModel_Package).Get(), &pActivationFactory);
    ASSERT(SUCCEEDED(result));
    if (SUCCEEDED(result))
    {
        result = pActivationFactory.As(&packageStatics);
        ASSERT(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        result = packageStatics->get_Current(&package);
        ASSERT(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        result = package->get_InstalledLocation(&storageFolder);
        ASSERT(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        result = storageFolder.As(&storageItem);
        ASSERT(SUCCEEDED(result));
    }

    if (SUCCEEDED(result))
    {
        result = storageItem->get_Path(hstrPath.GetAddressOf());
        ASSERT(SUCCEEDED(result));
    }
    
    if (SUCCEEDED(result))
    {
        try
        {
            std::wstring t = std::wstring(hstrPath.GetRawBuffer(nullptr));
            return std::string(t.begin(), t.end());
        }
        catch (std::bad_alloc)
        {
            result = E_OUTOFMEMORY;
        }
        ASSERT(SUCCEEDED(result));
    }

    UNREACHABLE();
    return std::string();
}

static float GetLogicalDpi()
{
    ComPtr<ABI::Windows::Graphics::Display::IDisplayPropertiesStatics> displayProperties;
    float dpi = 1.0f;

    if (SUCCEEDED(GetActivationFactory(HStringReference(RuntimeClass_Windows_Graphics_Display_DisplayProperties).Get(), displayProperties.GetAddressOf())))
    {
        if (SUCCEEDED(displayProperties->get_LogicalDpi(&dpi)))
        {
            return dpi;
        }
    }
    return 1.0f;
}

float convertDipsToPixels(float dips)
{
    static const float dipsPerInch = 96.0f;
    return floor(dips * GetLogicalDpi() / dipsPerInch + 0.5f); // Round to nearest integer.
}
};