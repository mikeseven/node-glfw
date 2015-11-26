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
        'VERSION=0.4.1',
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
        ['OS=="linux"', {
          'libraries': ['-lAntTweakBar', '<!@(pkg-config --libs glfw3 glew)']
        }],
        ['OS=="mac"', {
          'include_dirs': [ '<!@(pkg-config glfw3 glew --cflags-only-I | sed s/-I//g)','-I<(ANTTWEAKBAR_ROOT)/include'],
          'libraries': [ '<!@(pkg-config --libs glfw3 glew)', '-L<(ANTTWEAKBAR_ROOT)/lib', '-lAntTweakBar', '-framework OpenGL'],
          'library_dirs': ['/usr/local/lib'],
        }],
        ['OS=="win"', {
            'include_dirs': [
              './deps/include',
              ],
            'library_dirs': [
              './deps/windows/lib/<(target_arch)',
              ],
            'libraries': [
              'FreeImage.lib',
              'AntTweakBar64.lib',
              'glfw3dll.lib',
              'glew32.lib',
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
