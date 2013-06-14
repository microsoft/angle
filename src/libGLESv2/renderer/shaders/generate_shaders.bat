@ECHO OFF
REM
REM Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
REM Use of this source code is governed by a BSD-style license that can be
REM found in the LICENSE file.
REM

PATH %PATH%;%DXSDK_DIR%\Utilities\bin\x86

fxc /Gec /nologo /E standardvs /T vs_4_0_level_9_1 /Fh compiled/standardvs.h /Vn g_vs20_standardvs /Qstrip_reflect /Qstrip_debug Blit.vs
fxc /Gec /nologo /E flipyvs /T vs_4_0_level_9_1 /Fh compiled/flipyvs.h /Vn g_vs20_flipyvs /Qstrip_reflect /Qstrip_debug Blit.vs
fxc /Gec /nologo /E passthroughps /T ps_4_0_level_9_1 /Fh compiled/passthroughps.h /Vn g_ps20_passthroughps /Qstrip_reflect /Qstrip_debug Blit.ps
fxc /Gec /nologo /E luminanceps /T ps_4_0_level_9_1 /Fh compiled/luminanceps.h /Vn g_ps20_luminanceps /Qstrip_reflect /Qstrip_debug Blit.ps
fxc /Gec /nologo /E componentmaskps /T ps_4_0_level_9_1 /Fh compiled/componentmaskps.h /Vn g_ps20_componentmaskps /Qstrip_reflect /Qstrip_debug Blit.ps

fxc /Gec /nologo /E VS_Passthrough /T vs_4_0_level_9_1 /Fh compiled/passthrough11vs.h /Vn g_VS_Passthrough /Qstrip_reflect /Qstrip_debug Passthrough11.hlsl
fxc /Gec /nologo /E PS_PassthroughRGBA /T ps_4_0_level_9_1 /Fh compiled/passthroughrgba11ps.h /Vn g_PS_PassthroughRGBA /Qstrip_reflect /Qstrip_debug Passthrough11.hlsl
fxc /Gec /nologo /E PS_PassthroughRGB /T ps_4_0_level_9_1 /Fh compiled/passthroughrgb11ps.h /Vn g_PS_PassthroughRGB /Qstrip_reflect /Qstrip_debug Passthrough11.hlsl
fxc /Gec /nologo /E PS_PassthroughLum /T ps_4_0_level_9_1 /Fh compiled/passthroughlum11ps.h /Vn g_PS_PassthroughLum /Qstrip_reflect /Qstrip_debug Passthrough11.hlsl
fxc /Gec /nologo /E PS_PassthroughLumAlpha /T ps_4_0_level_9_1 /Fh compiled/passthroughlumalpha11ps.h /Vn g_PS_PassthroughLumAlpha /Qstrip_reflect /Qstrip_debug Passthrough11.hlsl

fxc /Gec /nologo /E VS_Clear /T vs_4_0_level_9_1 /Fh compiled/clear11vs.h /Vn g_VS_Clear /Qstrip_reflect /Qstrip_debug Clear11.hlsl
fxc /Gec /nologo /E PS_ClearSingle /T ps_4_0_level_9_1 /Fh compiled/clearsingle11ps.h /Vn g_PS_ClearSingle /Qstrip_reflect /Qstrip_debug Clear11.hlsl
fxc /Gec /nologo /E PS_ClearMultiple /T ps_4_0_level_9_1 /Fh compiled/clearmultiple11ps.h /Vn g_PS_ClearMultiple /Qstrip_reflect /Qstrip_debug Clear11.hlsl
