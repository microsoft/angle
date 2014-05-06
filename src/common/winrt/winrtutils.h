//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// winrtutils.h: Common Windows Runtime utilities.

#ifndef COMMON_WINRTUTILS_H_
#define COMMON_WINRTUTILS_H_

#include <string>

namespace winrt
{

std::string getTempPath();
float convertDipsToPixels(float dips);

};

#endif // COMMON_WINRTUTILS_H_

