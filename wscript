#
## This file contains proprietary software owned by Motorola Mobility, Inc. 
## No rights, expressed or implied, whatsoever to this software are provided by Motorola Mobility, Inc. hereunder. 
## 
## (c) Copyright 2011 Motorola Mobility, Inc.  All Rights Reserved.  
#

import Options
import sys
from os import unlink, symlink, popen, environ
from os.path import exists 

top='.'
srcdir = "."
blddir = "build"
VERSION = "0.1.0"

def set_options(opt):
  opt.tool_options('compiler_cxx')

def configure(conf):
  conf.check_tool("compiler_cxx")
  conf.check_tool('node_addon')
  conf.check(lib=['glfw'], uselib_store='GLFW')

def build(bld):
  obj = bld.new_task_gen("cxx", "shlib", "node_addon")
  obj.target = "node_glfw"
  obj.source  = bld.path.ant_glob('src/*.cc')
  obj.cxxflags = ["-g",'-I/opt/local/include','-fPIC']
  obj.uselib=['GLFW']
  if sys.platform.startswith('darwin'):
    obj.framework=['OpenGL']
  elif sys.platform.startswith('linux'):  
    obj.ldflags = [ "-lglfw","-lGL" ]

def shutdown():
  if Options.commands['clean']:
    if exists('node_glfw.node'): unlink('node_glfw.node')
  else:
    if exists('build/Release/node_glfw.node') and not exists('node_glfw.node'):
      symlink('build/Release/node_glfw.node', 'node_glfw.node')
