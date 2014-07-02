var glfw = require('../index');
var util=require('util');
var log = console.log;

var version = glfw.GetVersion();
log('glfw ' + version.major + '.' + version.minor + '.' + version.rev);
log('glfw version-string: ' + glfw.GetVersionString());

// Initialize GLFW
if (!glfw.Init()) {
  log("Failed to initialize GLFW");
  process.exit(-1);
}

// Open OpenGL window
//glfw.WindowHint(glfw.CONTEXT_VERSION_MAJOR, 3);
//glfw.WindowHint(glfw.CONTEXT_VERSION_MINOR, 2);
//glfw.WindowHint(glfw.OPENGL_PROFILE, glfw.OPENGL_ANY_PROFILE);
glfw.DefaultWindowHints();

var width=640, height=480;
var window=glfw.CreateWindow(width, height,"Test");
if (!window) {
  log("Failed to open GLFW window");
  glfw.Terminate();
  process.exit(-1);
}

glfw.MakeContextCurrent(window);

glfw.SetWindowTitle("GLFW Simple Test");

// testing events
glfw.events.on('keydown',function(evt) {
  log("[keydown] "+util.inspect(evt));
});

glfw.events.on('mousemove',function(evt) {
  log("[mousemove] "+evt.x+", "+evt.y);
});

glfw.events.on('mousewheel',function(evt) {
  log("[mousewheel] "+evt.position);
});

glfw.events.on('resize',function(evt){
  log("[resize] "+evt.width+", "+evt.height);
});

//can only be called after window creation!
var glVersion_major = glfw.GetWindowAttrib(window, glfw.CONTEXT_VERSION_MAJOR); 
var glVersion_minor = glfw.GetWindowAttrib(window, glfw.CONTEXT_VERSION_MINOR); 
var glVersion_rev = glfw.GetWindowAttrib(window, glfw.CONTEXT_REVISION); 
var glProfile = glfw.GetWindowAttrib(window, glfw.OPENGL_PROFILE); 
log('GL ' + glVersion_major + '.' + glVersion_minor + '.' + glVersion_rev+ " Profile: " + glProfile);

// Enable vertical sync (on cards that support it)
glfw.SwapInterval(0 /*1*/); // 0 for vsync off

var start = glfw.GetTime();
while(!glfw.WindowShouldClose(window) && !glfw.GetKey(window, glfw.KEY_ESCAPE)) {
  // Get window size (may be different than the requested size)
  var wsize = glfw.GetFramebufferSize(window);
  if(wsize) log("FB size: "+wsize.width+', '+wsize.height);

  glfw.testScene(wsize.width, wsize.height);
  
  // Swap buffers
  glfw.SwapBuffers(window);
  glfw.PollEvents();

}

// Close OpenGL window and terminate GLFW
glfw.DestroyWindow(window);
glfw.Terminate();

process.exit(0);
