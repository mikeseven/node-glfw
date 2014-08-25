{
  'variables': {
    'platform': '<(OS)',
  },
  'conditions': [
    # Replace gyp platform with node platform, blech
    ['platform == "mac"', {'variables': {'platform': 'darwin'}}],
    ['platform == "win"', {'variables': {'platform': 'win32'}}],
  ],
  'targets': [
    {
      'target_name': 'glfw',
      'defines': [
        'VERSION=0.3.1',
      ],
      'sources': [ 
        'src/atb.cc', 'src/glfw.cc'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        './deps/include',
      ],
      'conditions': [
        ['OS=="linux"', {'libraries': ['-lAntTweakBar', '<!@(pkg-config --libs glfw3 glew)']}],
        ['OS=="mac"', {
          'libraries': ['-lAntTweakBar', '-lglfw3', '-lGLEW', '-framework OpenGL'],
        }],
        ['OS=="win"', {
          'libraries': [
            'AntTweakBar64.lib',
            'glew64s.lib', 
            'glfw3dll.lib', 
            'opengl32.lib'
            ],
          'defines' : [
            'WIN32_LEAN_AND_MEAN',
            'VC_EXTRALEAN'
          ]
          },
        ],
      ],
    }
  ]
}
