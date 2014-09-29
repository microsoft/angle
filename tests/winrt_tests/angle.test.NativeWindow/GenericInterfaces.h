//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#pragma once

MIDL_INTERFACE("A4D67D0A-0E7F-4DF7-918B-7A1395413AF2")
IAmInspectable : public IInspectable
{

};

class GenericIInspectable : public RuntimeClass <
    RuntimeClassFlags<WinRtClassicComMix>,
    IAmInspectable >
{

};

MIDL_INTERFACE("3CBCFE7A-E000-4094-B2D2-B7C9A4D67A2C")
IAmUnknown : public IUnknown
{

};

class GenericIUnknown : public RuntimeClass <
    RuntimeClassFlags<ClassicCom>,
    IAmUnknown >
{

};
