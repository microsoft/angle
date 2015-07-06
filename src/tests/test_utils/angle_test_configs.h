//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TEST_CONFIGS_H_
#define ANGLE_TEST_CONFIGS_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "angle_test_instantiate.h"
#include "EGLWindow.h"

namespace angle
{

struct PlatformParameters
{
    PlatformParameters();
    PlatformParameters(EGLint majorVersion, EGLint minorVersion,
                       const EGLPlatformParameters &eglPlatformParameters);

    EGLint getRenderer() const;

    EGLint majorVersion;
    EGLint minorVersion;
    EGLPlatformParameters eglParameters;
};

<<<<<<< HEAD
inline std::ostream &operator<<(std::ostream& stream,
                                const PlatformParameters &pp)
{
    stream << "ES" << pp.mClientVersion << "_";

    switch (pp.mEGLPlatformParameters.renderer)
    {
      case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
        stream << "D3D9";
        break;
      case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
        stream << "D3D11";
        break;
      case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
        stream << "OPENGL";
        break;
      case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
        stream << "GLES";
        break;
      default:
        UNREACHABLE();
        break;
    }

    if (pp.mEGLPlatformParameters.majorVersion != EGL_DONT_CARE)
    {
        stream << "_" << pp.mEGLPlatformParameters.majorVersion;
    }

    if (pp.mEGLPlatformParameters.minorVersion != EGL_DONT_CARE)
    {
        stream << "_" << pp.mEGLPlatformParameters.minorVersion;
    }

    switch (pp.mEGLPlatformParameters.deviceType)
    {
      case EGL_DONT_CARE:
      case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
        // default
        break;

      case EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE:
        stream << "_REFERENCE";
        break;

      case EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE:
        stream << "_WARP";
        break;

      default:
        UNREACHABLE();
        break;
    }

    if (pp.mEGLPlatformParameters.useRenderToBackBuffer == EGL_TRUE)
    {
        stream << "_RTBB";
    }

    return stream;
}

inline std::vector<PlatformParameters> FilterPlatforms(const PlatformParameters *platforms, size_t numPlatforms)
{
    std::vector<PlatformParameters> filtered;

    for (size_t i = 0; i < numPlatforms; i++)
    {
        switch (platforms[i].mEGLPlatformParameters.renderer)
        {
          case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
#if defined(ANGLE_ENABLE_D3D9)
            filtered.push_back(platforms[i]);
#endif
            break;

          case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
#if defined(ANGLE_ENABLE_D3D11)
            filtered.push_back(platforms[i]);
#endif
            break;

          case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
          case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
#if defined(ANGLE_ENABLE_OPENGL)
            filtered.push_back(platforms[i]);
#endif
            break;

          default:
            UNREACHABLE();
            break;
        }
    }

    return filtered;
}

inline PlatformParameters ES2_D3D9()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D9_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_RTBB()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_TRUE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL11_0()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL10_1()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL10_0()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL9_3()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        9, 3,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_TRUE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL11_0_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL10_1_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL10_0_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL9_3_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        9, 3,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_TRUE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL11_0_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL10_1_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL10_0_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_D3D11_FL9_3_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        9, 3,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_TRUE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES3_D3D11()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL11_1()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL11_0()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL10_1()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL10_0()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL11_1_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL11_0_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL10_1_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL10_0_WARP()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_WARP_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL11_1_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL11_0_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL10_1_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_D3D11_FL10_0_REFERENCE()
{
    EGLPlatformParameters eglParams(
        EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
        10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_REFERENCE_ANGLE,
        EGL_FALSE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES2_OPENGL()
{
    EGLPlatformParameters eglParams(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES2_OPENGLES()
{
    EGLPlatformParameters eglParams(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
    return PlatformParameters(2, eglParams);
}

inline PlatformParameters ES3_OPENGL()
{
    EGLPlatformParameters eglParams(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE);
    return PlatformParameters(3, eglParams);
}

inline PlatformParameters ES3_OPENGLES()
{
    EGLPlatformParameters eglParams(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
    return PlatformParameters(3, eglParams);
}

#define ANGLE_INSTANTIATE_TEST(testName, ...) \
    const PlatformParameters testName##params[] = {__VA_ARGS__}; \
    INSTANTIATE_TEST_CASE_P(, testName, testing::ValuesIn(FilterPlatforms(testName##params, ArraySize(testName##params))));
=======
bool operator<(const PlatformParameters &a, const PlatformParameters &b);
bool operator==(const PlatformParameters &a, const PlatformParameters &b);
std::ostream &operator<<(std::ostream& stream, const PlatformParameters &pp);

// EGL platforms
namespace egl_platform
{

EGLPlatformParameters DEFAULT();
EGLPlatformParameters DEFAULT_NULL();

EGLPlatformParameters D3D9();
EGLPlatformParameters D3D9_NULL();
EGLPlatformParameters D3D9_REFERENCE();

EGLPlatformParameters D3D11();
EGLPlatformParameters D3D11_FL11_1();
EGLPlatformParameters D3D11_FL11_0();
EGLPlatformParameters D3D11_FL10_1();
EGLPlatformParameters D3D11_FL10_0();
EGLPlatformParameters D3D11_FL9_3();

EGLPlatformParameters D3D11_NULL();

EGLPlatformParameters D3D11_WARP();
EGLPlatformParameters D3D11_FL11_1_WARP();
EGLPlatformParameters D3D11_FL11_0_WARP();
EGLPlatformParameters D3D11_FL10_1_WARP();
EGLPlatformParameters D3D11_FL10_0_WARP();
EGLPlatformParameters D3D11_FL9_3_WARP();

EGLPlatformParameters D3D11_REFERENCE();
EGLPlatformParameters D3D11_FL11_1_REFERENCE();
EGLPlatformParameters D3D11_FL11_0_REFERENCE();
EGLPlatformParameters D3D11_FL10_1_REFERENCE();
EGLPlatformParameters D3D11_FL10_0_REFERENCE();
EGLPlatformParameters D3D11_FL9_3_REFERENCE();

EGLPlatformParameters OPENGL();
EGLPlatformParameters OPENGL(EGLint major, EGLint minor);
EGLPlatformParameters OPENGL_NULL();

EGLPlatformParameters OPENGLES();

} // namespace egl_platform

// ANGLE tests platforms
PlatformParameters ES2_D3D9();
PlatformParameters ES2_D3D9_REFERENCE();

PlatformParameters ES2_D3D11();
PlatformParameters ES2_D3D11_FL11_0();
PlatformParameters ES2_D3D11_FL10_1();
PlatformParameters ES2_D3D11_FL10_0();
PlatformParameters ES2_D3D11_FL9_3();

PlatformParameters ES2_D3D11_WARP();
PlatformParameters ES2_D3D11_FL11_0_WARP();
PlatformParameters ES2_D3D11_FL10_1_WARP();
PlatformParameters ES2_D3D11_FL10_0_WARP();
PlatformParameters ES2_D3D11_FL9_3_WARP();

PlatformParameters ES2_D3D11_REFERENCE();
PlatformParameters ES2_D3D11_FL11_0_REFERENCE();
PlatformParameters ES2_D3D11_FL10_1_REFERENCE();
PlatformParameters ES2_D3D11_FL10_0_REFERENCE();
PlatformParameters ES2_D3D11_FL9_3_REFERENCE();

PlatformParameters ES3_D3D11();
PlatformParameters ES3_D3D11_FL11_1();
PlatformParameters ES3_D3D11_FL11_0();
PlatformParameters ES3_D3D11_FL10_1();
PlatformParameters ES3_D3D11_FL10_0();

PlatformParameters ES3_D3D11_WARP();
PlatformParameters ES3_D3D11_FL11_1_WARP();
PlatformParameters ES3_D3D11_FL11_0_WARP();
PlatformParameters ES3_D3D11_FL10_1_WARP();
PlatformParameters ES3_D3D11_FL10_0_WARP();

PlatformParameters ES3_D3D11_REFERENCE();
PlatformParameters ES3_D3D11_FL11_1_REFERENCE();
PlatformParameters ES3_D3D11_FL11_0_REFERENCE();
PlatformParameters ES3_D3D11_FL10_1_REFERENCE();
PlatformParameters ES3_D3D11_FL10_0_REFERENCE();

PlatformParameters ES2_OPENGL();
PlatformParameters ES2_OPENGL(EGLint major, EGLint minor);
PlatformParameters ES3_OPENGL();
PlatformParameters ES3_OPENGL(EGLint major, EGLint minor);

PlatformParameters ES2_OPENGLES();
PlatformParameters ES3_OPENGLES();
>>>>>>> master

} // namespace angle

#endif // ANGLE_TEST_CONFIGS_H_
