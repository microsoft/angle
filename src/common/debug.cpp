//
// Copyright (c) 2002-2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// debug.cpp: Debugging utilities.

#include "common/debug.h"
#include "common/platform.h"
#include "common/angleutils.h"

#include <stdarg.h>
#include <vector>
#include <fstream>
#include <cstdio>

#if defined(_DEBUG) && defined(ANGLE_ENABLE_D3D11) && defined(ANGLE_ENABLE_WINDOWS_STORE)
#include <DXProgrammableCapture.h>
#include <dxgidebug.h>
#endif // ANGLE_ENABLE_D3D11 && ANGLE_ENABLE_WINDOWS_STORE

namespace gl
{
#if defined(ANGLE_ENABLE_PERF)
// Wraps the D3D9/D3D11 event marker functions.
class DebugEventWrapper
{
  public:
    DebugEventWrapper() { };
    virtual ~DebugEventWrapper() { };
    virtual void beginEvent(LPCWSTR wszName) = 0;
    virtual void endEvent() = 0;
    virtual void setMarker(LPCWSTR wszName) = 0;
    virtual bool getStatus() = 0;
};

#if defined(ANGLE_ENABLE_D3D9)
class D3D9DebugEventWrapper : public DebugEventWrapper
{ 
  public:
    void beginEvent(LPCWSTR wszName)
    {
        D3DPERF_BeginEvent(0, wszName);
    }

    void endEvent()
    {
        D3DPERF_EndEvent();
    }

    void setMarker(LPCWSTR wszName)
    {
        D3DPERF_SetMarker(0, wszName);
    }

    bool getStatus()
    {
        return !!D3DPERF_GetStatus();
    }
};
#elif defined(ANGLE_ENABLE_D3D11)
// If the project uses D3D9 then we can use the D3D9 event markers, even with the D3D11 renderer.
// However, if D3D9 is unavailable (e.g. in Windows Store), then we use D3D11 event markers.
// The D3D11 event markers are methods on ID3DUserDefinedAnnotation, which is implemented by the DeviceContext.
// This doesn't have to be the same DeviceContext that the renderer uses, though.
class D3D11DebugEventWrapper : public DebugEventWrapper
{
  public:

    D3D11DebugEventWrapper()
      : mInitialized(false),
        mD3d11Module(NULL),
        mUserDefinedAnnotation(NULL)
    {
        // D3D11 devices can't be created during DllMain.
        // We defer device creation until the object is actually used.
    }

    ~D3D11DebugEventWrapper()
    {
        if (mInitialized)
        {
            SafeRelease(mUserDefinedAnnotation);
            FreeLibrary(mD3d11Module);
        }
    }

    virtual void beginEvent(LPCWSTR wszName)
    {
        initializeDevice();

        mUserDefinedAnnotation->BeginEvent(wszName);
    }

    virtual void endEvent()
    {
        initializeDevice();

        mUserDefinedAnnotation->EndEvent();
    }

    virtual void setMarker(LPCWSTR wszName)
    {
        initializeDevice();

        mUserDefinedAnnotation->SetMarker(wszName);
    }

    virtual bool getStatus()
    {
        // ID3DUserDefinedAnnotation::GetStatus doesn't work with the Graphics Diagnostics tools in Visual Studio 2013.

#if defined(_DEBUG) && defined(ANGLE_ENABLE_WINDOWS_STORE)
        // In the Windows Store, we can use IDXGraphicsAnalysis. The call to GetDebugInterface1 only succeeds if the app is under capture.
        // This should only be called in DEBUG mode. 
        // If an app links against DXGIGetDebugInterface1 in release mode then it will fail Windows Store ingestion checks.
        IDXGraphicsAnalysis* graphicsAnalysis;
        DXGIGetDebugInterface1(0, IID_PPV_ARGS(&graphicsAnalysis));
        bool underCapture = (graphicsAnalysis != NULL);
        SafeRelease(graphicsAnalysis);
        return underCapture;
#endif
 
        // Otherwise, we have to return true here.
        return true;
    }

  protected:

    void initializeDevice()
    {
        if (!mInitialized)
        {
#if !defined(ANGLE_ENABLE_WINDOWS_STORE)
            mD3d11Module = LoadLibrary(TEXT("d3d11.dll"));
            ASSERT(mD3d11Module);

            PFN_D3D11_CREATE_DEVICE D3D11CreateDevice = (PFN_D3D11_CREATE_DEVICE)GetProcAddress(mD3d11Module, "D3D11CreateDevice");
            ASSERT(D3D11CreateDevice != NULL);
#endif // !ANGLE_ENABLE_WINDOWS_STORE

            ID3D11Device* device = NULL;
            ID3D11DeviceContext* context = NULL;
              
            HRESULT hr = E_FAIL;

            // Create a D3D_DRIVER_TYPE_NULL device, which is much cheaper than other types of device.
            hr =  D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &device, NULL, &context);
            ASSERT(SUCCEEDED(hr));
   
            hr = context->QueryInterface(__uuidof(mUserDefinedAnnotation), reinterpret_cast<void**>(&mUserDefinedAnnotation));
            ASSERT(SUCCEEDED(hr) && mUserDefinedAnnotation != NULL);

            SafeRelease(device);
            SafeRelease(context);
        
            mInitialized = true;
        }
    }

    bool mInitialized;
    HMODULE mD3d11Module;
    ID3DUserDefinedAnnotation* mUserDefinedAnnotation;
};
#endif // ANGLE_ENABLE_D3D9 elif ANGLE_ENABLE_D3D11

static DebugEventWrapper* g_DebugEventWrapper = NULL;

void InitializeDebugEvents()
{
#if defined(ANGLE_ENABLE_D3D9)
    g_DebugEventWrapper = new D3D9DebugEventWrapper();
#elif defined(ANGLE_ENABLE_D3D11)
    g_DebugEventWrapper = new D3D11DebugEventWrapper();
#endif
}

void UninitializeDebugEvents()
{
    if (g_DebugEventWrapper != NULL)
    {
        delete g_DebugEventWrapper;
        g_DebugEventWrapper = NULL;
    }
}

#endif // ANGLE_ENABLE_PERF

enum DebugTraceOutputType
{
   DebugTraceOutputTypeNone,
   DebugTraceOutputTypeSetMarker,
   DebugTraceOutputTypeBeginEvent
};

static void output(bool traceFileDebugOnly, DebugTraceOutputType outputType, const char *format, va_list vararg)
{
#if defined(ANGLE_ENABLE_PERF)
    static std::vector<char> buffer(512);

    if (perfActive())
    {
        int len = FormatStringIntoVector(format, vararg, &buffer);
        std::wstring formattedWideMessage(buffer.begin(), buffer.begin() + len);

        switch (outputType)
        {
            case DebugTraceOutputTypeNone:
                break;
            case DebugTraceOutputTypeBeginEvent:
                g_DebugEventWrapper->beginEvent(formattedWideMessage.c_str());
                break;
            case DebugTraceOutputTypeSetMarker:
                g_DebugEventWrapper->setMarker(formattedWideMessage.c_str());
                break;
        }
    }
#endif // ANGLE_ENABLE_PERF

#if defined(ANGLE_ENABLE_DEBUG_TRACE)
#if defined(NDEBUG)
    if (traceFileDebugOnly)
    {
        return;
    }
#endif // NDEBUG

    std::string formattedMessage = FormatString(format, vararg);

    static std::ofstream file(TRACE_OUTPUT_FILE, std::ofstream::app);
    if (file)
    {
        file.write(formattedMessage.c_str(), formattedMessage.length());
        file.flush();
    }

#if defined(ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER)
// Only output to the debugger window for debug builds only
#ifdef _DEBUG
    OutputDebugStringA(formattedMessage.c_str());
#endif // _DEBUG
#endif // ANGLE_ENABLE_DEBUG_TRACE_TO_DEBUGGER

#endif // ANGLE_ENABLE_DEBUG_TRACE

}

void trace(bool traceFileDebugOnly, const char *format, ...)
{
    va_list vararg;
    va_start(vararg, format);
#if defined(ANGLE_ENABLE_PERF)
    output(traceFileDebugOnly, DebugTraceOutputTypeSetMarker, format, vararg);
#else
    output(traceFileDebugOnly, DebugTraceOutputTypeNone, format, vararg);
#endif
    va_end(vararg);
}

bool perfActive()
{
#if defined(ANGLE_ENABLE_PERF)
    static bool active = g_DebugEventWrapper->getStatus();
    return active;
#else
    return false;
#endif
}

ScopedPerfEventHelper::ScopedPerfEventHelper(const char* format, ...)
{
#if !defined(ANGLE_ENABLE_DEBUG_TRACE)
    if (!perfActive())
    {
        return;
    }
#endif // !ANGLE_ENABLE_DEBUG_TRACE
    va_list vararg;
    va_start(vararg, format);
#if defined(ANGLE_ENABLE_PERF)
    output(true, DebugTraceOutputTypeBeginEvent, format, vararg);
#else
    output(true, DebugTraceOutputTypeNone, format, vararg);
#endif // ANGLE_ENABLE_PERF
    va_end(vararg);
}

ScopedPerfEventHelper::~ScopedPerfEventHelper()
{
#if defined(ANGLE_ENABLE_PERF)
    if (perfActive())
    {
        g_DebugEventWrapper->endEvent();
    }
#endif
}
}
