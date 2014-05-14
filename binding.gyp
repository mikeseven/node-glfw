{
  'variables': {
    'platform': '<(OS)',
    'glfw_version': 'GLFW3'
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
        'VERSION=0.2.0',
        '<(glfw_version)'
      ],
      'sources': [ 
        'src/atb.cc', 
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        './deps/include',
      ],
      'conditions': [
        ['glfw_version=="GLFW2"', {'sources' : ['src/glfw.cc']}],
        ['glfw_version=="GLFW3"', {'sources' : ['src/glfw3.cc']}],
        ['OS=="linux"', {'libraries': ['-lAntTweakBar', '<!@(pkg-config --libs glfw3 glew)']}],
        ['OS=="mac"', {
          'libraries': ['-lAntTweakBar', '<(module_root_dir)/libglfw3.a', '-lGLEW', '-framework OpenGL'],
        }],
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
