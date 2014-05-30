//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

#include "pch.h"

class EGLWrapper
{
public:
    EGLWrapper();
    void InitializeSurfacelessEGL(EGLNativeDisplayType displayId);
    void CleanupEGL();

private:
    EGLDisplay mDisplay;
    EGLContext mContext;
    EGLSurface mSurface;
};
