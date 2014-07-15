# Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
    'variables':
    {
        'component%': 'static_library',
        'angle_path%': '..',
        'windows_sdk_path%': '$(WindowsSdkDir)',
        'windows8_sdk_path%': 'C:/Program Files (x86)/Windows Kits/8.0',
        'windowsphone_sdk_path%': '$(WindowsPhoneSdkDir)',
    },
    'msvs_disabled_warnings': [ 4100, 4127, 4239, 4244, 4245, 4512, 4702, 4530, 4718, 4267, 4264, 4447, 4075 ],
    'msvs_system_include_dirs':
    [
        '<(windows_sdk_path)/Include/shared',
        '<(windows_sdk_path)/Include/um',
        '<(windows8_sdk_path)/Include/shared',
        '<(windows8_sdk_path)/Include/um',
    ],
    'msvs_settings':
    {
        'VCCLCompilerTool':
        {
            'PreprocessorDefinitions':
            [
                '_CRT_SECURE_NO_DEPRECATE',
                '_SCL_SECURE_NO_WARNINGS',
                '_HAS_EXCEPTIONS=0',
                'NOMINMAX',
            ],
        },
        'VCLinkerTool':
        {
            'AdditionalDependencies':
            [
                '%(AdditionalDependencies)',
                'uuid.lib',
            ],
        },
    },
    'configurations':
    {
        'Debug':
        {
            'msvs_settings':
            {
                'VCLinkerTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/win8/um/x86',
                        '<(windows8_sdk_path)/Lib/win8/um/x86',
                    ],
                },
                'VCLibrarianTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/win8/um/x86',
                        '<(windows8_sdk_path)/Lib/win8/um/x86',
                    ],
                },
            },
            'defines':
            [
                '_DEBUG'
            ],
        },
        'Release':
        {
            'msvs_settings':
            {
                'VCLinkerTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/win8/um/x86',
                        '<(windows8_sdk_path)/Lib/win8/um/x86',
                    ],
                },
                'VCLibrarianTool':
                {
                    'AdditionalLibraryDirectories':
                    [
                        '<(windows_sdk_path)/Lib/win8/um/x86',
                        '<(windows8_sdk_path)/Lib/win8/um/x86',
                    ],
                },
            },
            'defines':
            [
                'NDEBUG'
            ],
        },
    },
    'conditions':
    [
        ['component=="shared_library"',
        {
            'defines': [ 'COMPONENT_BUILD' ],
        }],
    ],
}
