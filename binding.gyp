{
  'targets': [
    {
      'target_name': 'node-glfw',
      'defines': [
        'VERSION=0.1.1'
      ],
      'sources': [ 'src/atb.cc', 'src/glfw.cc' ],
      'include_dirs': [
        '$(HOME)/code/AntTweakBar/include',
      ],
      'conditions': [
        ['OS=="mac"', {'libraries': ['-framework OpenGL']}],
      ],
      'libraries': [
        '-lAntTweakBar',
        '-lglfw',
        '-lGLEW',
      ],
    }
  ]
}
