var glfw = require('../index');
var util=require('util');
var log = console.log;

var version = glfw.GetVersion();
log('glfw ' + version.major + '.' + version.minor + '.' + version.rev);

// Initialize GLFW
if (!glfw.Init()) {
  log("Failed to initialize GLFW");
  process.exit(-1);
}

// Open OpenGL window
glfw.OpenWindowHint(glfw.OPENGL_MAJOR_VERSION, 3);
glfw.OpenWindowHint(glfw.OPENGL_MINOR_VERSION, 2);

if (!glfw.OpenWindow(640, 480, 0, 0, 0, 0, // r,g,b,a bits
0, 0, // depth, stencil bits
glfw.WINDOW)) {
  log("Failed to open GLFW window");
  glfw.Terminate();
  process.exit(-1);
}

glfw.SetWindowTitle("Trilinear interpolation");

// testing events
glfw.events.on('keydown',function(evt){
  log("[keydown] "+util.inspect(evt));
});

glfw.events.on('mousemove',function(evt){
  log("[mousemove] "+evt.x+", "+evt.y);
});

glfw.events.on('mousewheel',function(evt){
  log("[mousewheel] "+evt.position);
});

glfw.events.on('resize',function(evt){
  log("[resize] "+evt.width+", "+evt.height);
});

var glVersion = glfw.GetGLVersion(); // can only be called after window creation!
log('gl ' + glVersion.major + '.' + glVersion.minor + '.' + glVersion.rev);

// Enable sticky keys
glfw.Enable(glfw.STICKY_KEYS);

// Enable vertical sync (on cards that support it)
glfw.SwapInterval(0 /*1*/); // 0 for vsync off

var start = glfw.GetTime();
do {
  // Get time and mouse position
  var end = glfw.GetTime();
  var delta = end - start;
  start = end;

  //log('time: '+(delta*1000)+'ms');

  var mouse = glfw.GetMousePos();
  //log("mouse: "+mouse.x+', '+mouse.y);

  // Get window size (may be different than the requested size)
  var wsize = glfw.GetWindowSize();
  //log("window size: "+wsize.width+', '+wsize.height);

  // Swap buffers
  glfw.SwapBuffers();

} while (!glfw.GetKey(glfw.KEY_ESC) && glfw.GetWindowParam(glfw.OPENED));

// Close OpenGL window and terminate GLFW
glfw.Terminate();

process.exit(0);
