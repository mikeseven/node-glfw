var glfw = require('../index');
var util = require('util');
var log = console.log;

var version = glfw.GetVersion();
log('glfw ' + version.major + '.' + version.minor + '.' + version.rev);

// Initialize GLFW
if (!glfw.Init()) {
  log("Failed to initialize GLFW");
  process.exit(-1);
}

// Open OpenGL window
//glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, 3);
//glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, 2);
//glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_ANY_PROFILE);

var width=640, height=480;
var window=glfw.CreateWindow(width, height,"Test");

if (!window) {
  log("Failed to open GLFW window");
  glfw.Terminate();
  process.exit(-1);
}

glfw.SetWindowTitle("Trilinear interpolation");

// testing events
glfw.events.on('keydown', function(evt) {
  log("[keydown] " + util.inspect(evt));
});

glfw.events.on('mousemove', function(evt) {
  log("[mousemove] " + evt.x + ", " + evt.y);
});

glfw.events.on('scroll', function(evt) {
  log("[scroll] " + evt.xoffset + ", " + evt.yoffset);
});

glfw.events.on('resize', function(evt) {
  log("[resize] " + (width=evt.width) + ", " + (height=evt.height));
});

//can only be called after window creation!
var glVersion_major = glfw.GetWindowAttrib(window, glfw.CONTEXT_VERSION_MAJOR); 
var glVersion_minor = glfw.GetWindowAttrib(window, glfw.CONTEXT_VERSION_MINOR); 
var glVersion_rev = glfw.GetWindowAttrib(window, glfw.CONTEXT_REVISION); 
var glProfile = glfw.GetWindowAttrib(window, glfw.OPENGL_PROFILE); 
log('GL ' + glVersion_major + '.' + glVersion_minor + '.' + glVersion_rev+ " Profile: " + glProfile);

//// Enable sticky keys
//glfw.Enable(glfw.STICKY_KEYS);

// Enable vertical sync (on cards that support it)
glfw.MakeContextCurrent(window);
glfw.SwapInterval( 1 ); // 0 for vsync off

var start = glfw.GetTime();
do {
  // Get time and mouse position
  var end = glfw.GetTime();
  var delta = end - start;
  start = end;

  // log('time: '+(delta*1000)+'ms');

  //var mouse = glfw.GetCursorPos(window);
  //if(mouse) log("mouse: "+mouse.x+', '+mouse.y);

  // Get window size (may be different than the requested size)
  //var wsize = glfw.GetWindowSize(window);
  //if(wsize) log("window size: "+wsize.width+', '+wsize.height);

  glfw.testScene(width, height);
  
  // Swap buffers
  glfw.SwapBuffers(window);
  glfw.PollEvents();

} while (!glfw.GetKey(window, glfw.KEY_ESCAPE) && !glfw.WindowShouldClose(window));

// Close OpenGL window and terminate GLFW
glfw.Terminate();

process.exit(0);
