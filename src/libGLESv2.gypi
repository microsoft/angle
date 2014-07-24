# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'angle_enable_d3d9%': 1,
        'angle_enable_d3d11%': 1,
    },
    'target_defaults':
    {
        'defines':
        [
            'ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES={ TEXT("d3dcompiler_47.dll"), TEXT("d3dcompiler_46.dll"), TEXT("d3dcompiler_43.dll") }',
        ],
        'conditions':
        [
            ['angle_build_winrt==1',
            {
                'defines':
                [
                    'ANGLE_PRELOADED_D3DCOMPILER_MODULE_NAMES={ TEXT("d3dcompiler_47.dll"), TEXT("d3dcompiler_46.dll"), TEXT("d3dcompiler_43.dll") }',
                    'ANGLE_ENABLE_WINDOWS_STORE',
                    'NTDDI_VERSION=NTDDI_WINBLUE',
                    'ANGLE_ENABLE_RENDER_TO_BACK_BUFFER',
                ],
            }],
            ['angle_build_winphone==1',
            {
                'msvs_enable_winphone' : '1',
            }],
        ],
    },

    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'libGLESv2',
                    'type': 'shared_library',
                    'dependencies': [ 'translator', 'commit_id', 'copy_compiler_dll' ],
                    'includes': [ '../build/common_defines.gypi', ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                        'libGLESv2',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_path)/enumerate_files.py \
                             -dirs common libGLESv2 third_party/murmurhash ../include third_party/systeminfo \
                             -types *.cpp *.h *.hlsl *.vs *.ps *.bat *.def *.rc \
                             -excludes */d3d/* */d3d9/* */d3d11/* */winrt/* */win32/*)',
                    ],
                    'defines':
                    [
                        'GL_APICALL=',
                        'GL_GLEXT_PROTOTYPES=',
                        'EGLAPI=',
                    ],
                    'conditions':
                    [
                        ['angle_enable_d3d9==1',
                        {
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py \
                                     -dirs libGLESv2/renderer/d3d libGLESv2/renderer/d3d9 \
                                     -types *.cpp *.h *.vs *.ps *.bat)',
                            ],
                            'defines':
                            [
                                'ANGLE_ENABLE_D3D9',
                            ],
                            'msvs_settings':
                            {
                                'VCLinkerTool':
                                {
                                    'AdditionalDependencies':
                                    [
                                        'd3d9.lib',
                                    ]
                                }
                            },
                        }],
                        ['angle_enable_d3d11==1',
                        {
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py \
                                     -dirs libGLESv2/renderer/d3d libGLESv2/renderer/d3d11 \
                                     -types *.cpp *.h *.hlsl *.bat)',
                            ],
                            'defines':
                            [
                                'ANGLE_ENABLE_D3D11',
                            ],
                            'msvs_settings':
                            {
                                'VCLinkerTool':
                                {
                                    'AdditionalDependencies':
                                    [
                                        'dxguid.lib',
                                        'd3d11.lib',
                                        'd3dcompiler.lib',
                                    ],
                                },
                            },
                        }],
                        ['angle_build_winrt==0',
                        {
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py \
                                     -dirs common/win32 \
                                     -types *.cpp *.h )',
                            ],
                        }],
                        ['angle_build_winrt==1',
                        {
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py \
                                     -dirs common/winrt third_party/threademulation \
                                     -types *.cpp *.h )',
                            ],
                            'msvs_enable_winrt' : '1',
                            'msvs_requires_importlibrary' : 'true',
                            'msvs_settings':
                            {
                                'VCLinkerTool':
                                {
                                    'EnableCOMDATFolding': '1',
                                    'OptimizeReferences': '1',
                                }
                            },
                        }],
                        ['angle_build_winphone==1',
                        {
                            'msvs_enable_winphone' : '1',
                        }],
                    ],

                    'configurations':
                    {
                        'Debug':
                        {
                            'defines':
                            [
                                'ANGLE_ENABLE_PERF',
                            ],
                            'msvs_settings':
                            {
                                'VCLinkerTool':
                                {
                                    'AdditionalDependencies':
                                    [
                                        'd3d11.lib',
                                        'd3dcompiler.lib',
                                    ]
                                }
                            },
                        },
                    },
                },
            ],
        },
        ],
    ],
}
