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
      #'target_name': 'glfw-<(platform)-<(target_arch)',
      'target_name': 'glfw',
      'defines': [
        'VERSION=0.1.2'
      ],
      'sources': [ 'src/atb.cc', 'src/glfw.cc' ],
      'include_dirs': [
        './deps/include',
      ],
      'library_dirs': [
        './deps/<(platform)',
      ],
      'conditions': [
        ['OS=="linux"', {'libraries': ['-lAntTweakBar', '-lglfw', '-lGLEW']}],
        ['OS=="mac"', {'libraries': ['-lAntTweakBar', '-lglfw', '-lGLEW', '-framework OpenGL']}],
        ['OS=="win"', {
          'libraries': [
            'AntTweakBar64.lib',
            'glew64s.lib', 
            'glfw64dll.lib', 
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
