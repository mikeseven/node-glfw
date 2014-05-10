var glfw = require('../index');
var util=require('util');
var log = console.log;
var ATB= new glfw.AntTweakBar();

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

var width=640, height=480;

if (!glfw.OpenWindow(width, height, 0, 0, 0, 0, // r,g,b,a bits
                      0, 0, // depth, stencil bits
                      glfw.WINDOW)) 
{
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
  width=evt.width;
  height=evt.height;
  log("[resize] "+width+", "+height);
});

var glVersion = glfw.GetGLVersion(); // can only be called after window creation!
log('OpenGL ' + glVersion.major + '.' + glVersion.minor + '.' + glVersion.rev);

// Enable sticky keys
glfw.Enable(glfw.STICKY_KEYS);

// Enable vertical sync (on cards that support it)
glfw.SwapInterval(0 /*1*/); // 0 for vsync off

log('init AntTweakBar');
log(util.inspect(ATB,{ showHidden: true, depth: null }));

ATB.Init();
ATB.Define(" GLOBAL help='This example shows how to integrate AntTweakBar with GLFW and OpenGL.' "); // Message added to the help bar.
ATB.WindowSize(width, height);

twBar=new ATB.NewBar("Cube");
var z=0.5;
twBar.AddVar("z", ATB.TYPE_FLOAT, {
  getter: function(data){ return z; },
  setter: function(val,data) { z=val; log(z); },
},
" label='z' min=-1 max=1 step=0.01 keyIncr=s keyDecr=S help='Eye distance' ");

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

  glfw.testScene(wsize.width, wsize.height,z);

  ATB.Draw();

  // Swap buffers
  glfw.SwapBuffers();

} while (!glfw.GetKey(glfw.KEY_ESC) && glfw.GetWindowParam(glfw.OPENED));

// Close OpenGL window and terminate GLFW
glfw.Terminate();

process.exit(0);
