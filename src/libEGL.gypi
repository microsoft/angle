# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'conditions':
    [
        ['OS=="win"',
        {
            'targets':
            [
                {
                    'target_name': 'libEGL',
                    'type': 'shared_library',
                    'dependencies': [ 'libGLESv2', 'commit_id' ],
                    'include_dirs':
                    [
                        '.',
                        '../include',
                        'libGLESv2',
                    ],
                    'sources':
                    [
                        '<!@(python <(angle_path)/enumerate_files.py \
                             -dirs common libEGL ../include \
                             -types *.cpp *.h *.def *.rc \
                             -excludes */winrt/* */win32/*)',
                    ],
                    'defines':
                    [
                        'GL_APICALL=',
                        'GL_GLEXT_PROTOTYPES=',
                        'EGLAPI=',
                    ],
                    'conditions':
                    [
                        ['angle_build_winrt==0',
                        {
                            'defines':
                            [
                                'GL_APICALL=',
                                'GL_GLEXT_PROTOTYPES=',
                                'EGLAPI=',
                            ],
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py \
                                     -dirs common/win32 \
                                     -types *.cpp *.h )',
                            ],
                        }],
                        ['angle_build_winrt==1',
                        {
                            'defines':
                            [
                                'GL_APICALL=',
                                'GL_GLEXT_PROTOTYPES=',
                                'EGLAPI=',
                                'NTDDI_VERSION=NTDDI_WINBLUE',
                                'ANGLE_ENABLE_RENDER_TO_BACK_BUFFER',
                            ],
                            'sources':
                            [
                                '<!@(python <(angle_path)/enumerate_files.py \
                                     -dirs common/winrt third_party/threademulation \
                                     -types *.cpp *.h )',
                            ],
                            'msvs_enable_winrt' : '1',
                            'msvs_requires_importlibrary' : '1',
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
                    'includes': [ '../build/common_defines.gypi', ],
                    'msvs_settings':
                    {
                        'VCLinkerTool':
                        {
                            'conditions':
                            [
                                ['angle_build_winrt==0',
                                {
                                    'AdditionalDependencies':
                                    [
                                        'd3d9.lib',
                                    ],
                                }],
                            ],
                        },
                    },
                    'configurations':
                    {
                        'Debug':
                        {
                            'defines':
                            [
                                'ANGLE_ENABLE_PERF',
                            ],
                        },
                    },
                },
            ],
        },
        ],
    ],
}
