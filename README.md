NodeJS bindings to GLFW
=======================

This projects attempts to provide platform-independent access to windowing system and input devices on desktop/laptop computers using GLFW 3 and above.

Dependencies
------------
- NodeJS
While v0.6.5+ work in many cases, some missing features for typed arrays are only available in v0.7.x. So we recommend at least v0.7.5.

- GLEW (http://glew.sourceforge.net/)
GLEW is used to find OpenGL extensions in a cross-platform manner.

- GLFW (http://www.glfw.org/)
GLFW is a simple multi-platform framework for opening a window, creating an OpenGL context, and managing input.

- AntTweakBar (http://www.antisphere.com/Wiki/tools:anttweakbar)
AntTweakBar allows programmers to quickly add a light and intuitive graphical user interface into graphic applications to interactively tweak parameters on-screen. 

All of these libraries are cross-platform. node-glfw provides a Javascript wrapper to access native methods in GLFW and AntTweakBar. See example of usage in node-webgl/test/cube.js.

Installation (Mac, Linux)
-------------------------
Install GLFW and make sure examples are working. For convenience, install GLFW into your system lib/include path.

Install AntTweakBar and make sure its GLFW samples are working. For convenience, install AntTweakBar libraries in your system lib path.

Install GLEW and make sure its tests programs are working such as visualinfo. You should install GLEW in your system lib/include path.

Now you can install node-glfw, the usual way: npm install node-glfw.

Installation (Windows)
-------------------------
Copy all deps/*.lib into your <Visual Studio>/VC/lib for 32-bit libraries, and <Visual Studio>/VC/lib/amd64 for 64-bit libraries.
Copy all deps/*.dll into <Windows>/System32.

Notes
-----
- node-glfw is a just a platform binding so don't expect samples here. You should install node-webgl, which contains lots of tests and examples using node-glfw features including AntTweakBar. See node-webgl/test/cube.js for an example of using AntTweakBar with your webgl code.


