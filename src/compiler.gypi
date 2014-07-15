# Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'targets':
    [
        {
            'target_name': 'preprocessor',
            'type': 'static_library',
            'includes': [ '../build/common_defines.gypi', ],
            'sources': [ '<!@(python <(angle_path)/enumerate_files.py compiler/preprocessor -types *.cpp *.h *.y *.l )' ],
            'conditions':
            [
                ['angle_build_winrt==1',
                {
                    'msvs_enable_winrt' : '1',
                }],
                ['angle_build_winphone==1',
                {
                    'msvs_enable_winphone' : '1',
                }],
            ],
        },
        {
            'target_name': 'translator_lib',
            'type': 'static_library',
            'dependencies': [ 'preprocessor' ],
            'includes': [ '../build/common_defines.gypi', ],
            'include_dirs':
            [
                '.',
                '../include',
            ],
            'sources':
            [
                '<!@(python <(angle_path)/enumerate_files.py \
                     -dirs compiler/translator third_party/compiler common ../include \
                     -excludes compiler/translator/ShaderLang.cpp */winrt/* */win32/*\
                     -types *.cpp *.h *.y *.l)',
            ],
            'msvs_settings':
            {
              'VCLibrarianTool':
              {
                'AdditionalOptions': ['/ignore:4221']
              },
            },
            'conditions':
            [
                ['angle_build_winrt==1',
                {
                    'msvs_enable_winrt' : '1',
                }],
                ['angle_build_winphone==1',
                {
                    'msvs_enable_winphone' : '1',
                }],
            ],
        },

        {
            'target_name': 'translator',
            'type': '<(component)',
            'dependencies': [ 'translator_lib' ],
            'includes': [ '../build/common_defines.gypi', ],
            'include_dirs':
            [
                '.',
                '../include',
            ],
            'defines':
            [
                'ANGLE_TRANSLATOR_IMPLEMENTATION',
            ],
            'conditions':
            [
                ['angle_build_winrt==1',
                {
                    'defines':
                    [
                        'ANGLE_TRANSLATOR_IMPLEMENTATION',
                        'ANGLE_ENABLE_WINDOWS_STORE',
                    ],
                    'msvs_enable_winrt' : '1',
                }],
                ['angle_build_winphone==1',
                {
                    'msvs_enable_winphone' : '1',
                }],
            ],
            'sources':
            [
                'compiler/translator/ShaderLang.cpp'
            ],
        },

        {
            'target_name': 'translator_static',
            'type': 'static_library',
            'dependencies': [ 'translator_lib' ],
            'includes': [ '../build/common_defines.gypi', ],
            'include_dirs':
            [
                '.',
                '../include',
            ],
            'defines':
            [
                'ANGLE_TRANSLATOR_STATIC',
            ],
            'direct_dependent_settings':
            {
                'defines':
                [
                    'ANGLE_TRANSLATOR_STATIC',
                ],
            },
            'sources':
            [
                'compiler/translator/ShaderLang.cpp'
            ],
            'conditions':
            [
                ['angle_build_winrt==1',
                {
                    'msvs_enable_winrt' : '1',
                }],
                ['angle_build_winphone==1',
                {
                    'msvs_enable_winphone' : '1',
                }],
            ],
        },
    ],
}
