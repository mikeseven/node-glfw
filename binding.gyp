{
  'variables': {
    'platform': '<(OS)',
  },
  'conditions': [
    # Replace gyp platform with node platform, blech
    ['platform == "mac"', {'variables': {
      'platform': 'darwin',
      'ANTTWEAKBAR_ROOT': '/usr/local/Cellar/anttweakbar/1.16',
    }}],
    ['platform == "win"', {'variables': {'platform': 'win32'}}],
  ],
  'targets': [
    {
      'target_name': 'glfw',
      'defines': [
        'VERSION=0.4.0',
      ],
      'sources': [
        'src/atb.cc',
        'src/glfw.cc'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        './deps/include',
      ],
      'conditions': [
        ['OS=="linux"', {'libraries': ['-lAntTweakBar', '<!@(pkg-config --libs glfw3 glew)']}],
        ['OS=="mac"', {
          'include_dirs': [ '<!@(pkg-config glfw3 glew --cflags-only-I | sed s/-I//g)','-I<(ANTTWEAKBAR_ROOT)/include'],
          'libraries': [ '<!@(pkg-config --libs glfw3 glew)', '-L<(ANTTWEAKBAR_ROOT)/lib', '-lAntTweakBar', '-framework OpenGL']
        }],
        ['OS=="win"', {
            'include_dirs': [
              './deps/glew/include',
              './deps/glfw/include',
              './deps/AntTweakBar/include'
              ],
            'library_dirs': [
              './deps/glew/windows/lib/<(target_arch)',
              './deps/glfw/windows/lib-msvc120/<(target_arch)',
              './deps/AntTweakBar/windows/lib/'
              ],
            'libraries': [
              'glew32.lib',
              'glfw3dll.lib',
              'opengl32.lib'
              ],
            'defines' : [
              'WIN32_LEAN_AND_MEAN',
              'VC_EXTRALEAN'
            ],
            'cflags' : [
              '/O2','/Oy','/GL','/GF','/Gm-','/EHsc','/MT','/GS','/Gy','/GR-','/Gd'
            ],
            'ldflags' : [
              '/OPT:REF','/OPT:ICF','/LTCG'
            ],
            'conditions': [
              ['target_arch=="ia32"', {
                'libraries': ['AntTweakBar.lib']
              }],
              ['target_arch=="x64"', {
                'libraries': ['AntTweakBar64.lib']
              }]
            ]
          },
        ],
      ],
    }
  ]
}
