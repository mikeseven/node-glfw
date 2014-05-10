#include "common.h"
#include "atb.h"

// Includes
#include <cstdio>
#include <cstdlib>

using namespace v8;
using namespace node;

#include <iostream>
using namespace std;

namespace glfw {

/* GLFW initialization, termination and version querying */

NAN_METHOD(Init) {
  NanScope();
  NanReturnValue(JS_BOOL(glfwInit()==1));
}

NAN_METHOD(Terminate) {
  NanScope();
  glfwTerminate();
  NanReturnUndefined();
}

NAN_METHOD(GetVersion) {
  NanScope();
  int major, minor, rev;
  glfwGetVersion(&major,&minor,&rev);
  Local<Array> arr=Array::New(3);
  arr->Set(JS_STR("major"),JS_INT(major));
  arr->Set(JS_STR("minor"),JS_INT(minor));
  arr->Set(JS_STR("rev"),JS_INT(rev));
  NanReturnValue(arr);
}

/* Window handling */
Persistent<Object> glfw_events;
int lastX=0,lastY=0;
bool windowCreated=false;

void NAN_INLINE(CallEmitter(int argc, Handle<Value> argv[])) {
  NanScope();
  // MakeCallback(glfw_events, "emit", argc, argv);
  if(NanPersistentToLocal(glfw_events)->Has(NanSymbol("emit"))) {
    Local<Function> callback = NanPersistentToLocal(glfw_events)->Get(NanSymbol("emit")).As<Function>();

    if (!callback.IsEmpty()) {
      callback->Call(Context::GetCurrent()->Global(),argc,argv);
    }
  }
}

void APIENTRY keyCB(int key, int action) {
  if(!TwEventKeyGLFW(key,action)) {
    NanScope();

    Local<Array> evt=Array::New(7);
    evt->Set(JS_STR("type"),JS_STR(action ? "keydown" : "keyup"));
    evt->Set(JS_STR("ctrlKey"),JS_BOOL(glfwGetKey(GLFW_KEY_LCTRL) || glfwGetKey(GLFW_KEY_RCTRL)));
    evt->Set(JS_STR("shiftKey"),JS_BOOL(glfwGetKey(GLFW_KEY_LSHIFT) || glfwGetKey(GLFW_KEY_RSHIFT)));
    evt->Set(JS_STR("altKey"),JS_BOOL(glfwGetKey(GLFW_KEY_LALT) || glfwGetKey(GLFW_KEY_RALT)));
    evt->Set(JS_STR("metaKey"),JS_BOOL(glfwGetKey(GLFW_KEY_LSUPER) || glfwGetKey(GLFW_KEY_RSUPER)));

    if(key==GLFW_KEY_ESC) key=27;
    else if(key==GLFW_KEY_LSHIFT || key==GLFW_KEY_RSHIFT) key=16;
    else if(key==GLFW_KEY_LCTRL || key==GLFW_KEY_RCTRL) key=17;
    else if(key==GLFW_KEY_LALT || key==GLFW_KEY_RALT) key=18;
    else if(key==GLFW_KEY_LSUPER) key=91;
    else if(key==GLFW_KEY_RSUPER) key=93;
    evt->Set(JS_STR("which"),JS_INT(key));
    evt->Set(JS_STR("keyCode"),JS_INT(key));
    evt->Set(JS_STR("charCode"),JS_INT(key));

    Handle<Value> argv[2] = {
      JS_STR(action ? "keydown" : "keyup"), // event name
      evt
    };

    CallEmitter(2, argv);
  }
}

void APIENTRY mousePosCB(int x, int y) {
  if(!TwEventMousePosGLFW(x,y)) {
    int w,h;
    glfwGetWindowSize(&w, &h);
    if(x<0 || x>=w) return;
    if(y<0 || y>=h) return;

    lastX=x;
    lastY=y;

    NanScope();

    Local<Array> evt=Array::New(5);
    evt->Set(JS_STR("type"),JS_STR("mousemove"));
    evt->Set(JS_STR("pageX"),JS_INT(x));
    evt->Set(JS_STR("pageY"),JS_INT(y));
    evt->Set(JS_STR("x"),JS_INT(x));
    evt->Set(JS_STR("y"),JS_INT(y));

    Handle<Value> argv[2] = {
      JS_STR("mousemove"), // event name
      evt
    };

    CallEmitter(2, argv);
  }
}

void APIENTRY windowSizeCB(int w, int h) {
  NanScope();
  //cout<<"resizeCB: "<<w<<" "<<h<<endl;

  Local<Array> evt=Array::New(3);
  evt->Set(JS_STR("type"),JS_STR("resize"));
  evt->Set(JS_STR("width"),JS_INT(w));
  evt->Set(JS_STR("height"),JS_INT(h));

  Handle<Value> argv[2] = {
    JS_STR("resize"), // event name
    evt
  };

  CallEmitter(2, argv);
}

void APIENTRY mouseButtonCB(int button, int action) {
  if(!TwEventMouseButtonGLFW(button,action)) {
    NanScope();
    Local<Array> evt=Array::New(7);
    evt->Set(JS_STR("type"),JS_STR(action ? "mousedown" : "mouseup"));
    evt->Set(JS_STR("button"),JS_INT(button));
    evt->Set(JS_STR("which"),JS_INT(button));
    evt->Set(JS_STR("x"),JS_INT(lastX));
    evt->Set(JS_STR("y"),JS_INT(lastY));
    evt->Set(JS_STR("pageX"),JS_INT(lastX));
    evt->Set(JS_STR("pageY"),JS_INT(lastY));

    Handle<Value> argv[2] = {
      JS_STR(action ? "mousedown" : "mouseup"), // event name
      evt
    };

    CallEmitter(2, argv);
  }
}

void APIENTRY mouseWheelCB(int pos) {
  if(!TwEventMouseWheelGLFW(pos)) {
    NanScope();

    Local<Array> evt=Array::New(2);
    evt->Set(JS_STR("type"),JS_STR("mousewheel"));
    evt->Set(JS_STR("wheelDelta"),JS_INT(pos));

    Handle<Value> argv[2] = {
      JS_STR("mousewheel"), // event name
      evt
    };

    CallEmitter(2, argv);
  }
}

void APIENTRY windowRefreshCB() {
  #ifdef LOGGING
  cout<<"windowRefreshCB"<<endl;
  #endif
}

int APIENTRY windowCloseCB() {
  NanScope();

  Handle<Value> argv[1] = {
    JS_STR("quit"), // event name
  };

  CallEmitter(1, argv);

  return 1;
}

NAN_METHOD(testScene) {
  NanScope();
  int width = args[0]->Uint32Value();
  int height = args[1]->Uint32Value();
  float ratio = width / (float) height;

  glViewport(0, 0, width, height);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);

  glBegin(GL_TRIANGLES);
  glColor3f(1.f, 0.f, 0.f);
  glVertex3f(-0.6f, -0.4f, 0.f);
  glColor3f(0.f, 1.f, 0.f);
  glVertex3f(0.6f, -0.4f, 0.f);
  glColor3f(0.f, 0.f, 1.f);
  glVertex3f(0.f, 0.6f, 0.f);
  glEnd();

  NanReturnValue(Undefined());
}

NAN_METHOD(OpenWindow) {
  NanScope();
  int width       = args[0]->Uint32Value();
  int height      = args[1]->Uint32Value();
  int redbits     = args[2]->Uint32Value();
  int greenbits   = args[3]->Uint32Value();
  int bluebits    = args[4]->Uint32Value();
  int alphabits   = args[5]->Uint32Value();
  int depthbits   = args[6]->Uint32Value();
  int stencilbits = args[7]->Uint32Value();
  int mode        = args[8]->Uint32Value();

  if(!windowCreated) {
    /*GLFWvidmode vmode;   // GLFW video mode
    glfwGetDesktopMode(&vmode);
    windowCreated=glfwOpenWindow(width,height, vmode.RedBits, vmode.GreenBits, vmode.BlueBits,
        alphabits,depthbits,stencilbits, mode);*/
    windowCreated=glfwOpenWindow(width,height,redbits,greenbits,bluebits,alphabits,depthbits,stencilbits,mode)!=0;

    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
  }
  else
    glfwSetWindowSize(width,height);

  // Set callback functions
  NanInitPersistent(Object,_events,args.This()->Get(JS_STR("events"))->ToObject());
  NanAssignPersistent(Object, glfw_events, _events);

  glfwSetWindowSizeCallback( (GLFWwindowsizefun) windowSizeCB );
  glfwSetWindowRefreshCallback( (GLFWwindowrefreshfun) windowRefreshCB );
  glfwSetWindowCloseCallback( (GLFWwindowclosefun) windowCloseCB );
  glfwSetMousePosCallback( (GLFWmouseposfun) mousePosCB );
  glfwSetMouseButtonCallback( (GLFWmousebuttonfun) mouseButtonCB );
  glfwSetMouseWheelCallback( (GLFWmousewheelfun) mouseWheelCB );
  glfwSetKeyCallback( (GLFWkeyfun)keyCB);

  //glfwSetMouseButtonCallback((GLFWmousebuttonfun)TwEventMouseButtonGLFW);
  //glfwSetMousePosCallback((GLFWmouseposfun)TwEventMousePosGLFW);
  //glfwSetMouseWheelCallback((GLFWmousewheelfun)TwEventMouseWheelGLFW);
  //glfwSetKeyCallback((GLFWkeyfun)TwEventKeyGLFW);
  glfwSetCharCallback((GLFWcharfun)TwEventCharGLFW);

  //testScene(width,height);

  NanReturnValue(JS_BOOL(windowCreated));
}

NAN_METHOD(OpenWindowHint) {
  NanScope();
  glfwOpenWindowHint(args[0]->Int32Value(),args[1]->Int32Value());
  NanReturnUndefined();
}

NAN_METHOD(CloseWindow) {
  NanScope();
  glfwCloseWindow();
  NanReturnUndefined();
}

NAN_METHOD(SetWindowTitle) {
  NanScope();
  String::Utf8Value str(args[0]->ToString());
  glfwSetWindowTitle(*str);
  NanReturnUndefined();
}

NAN_METHOD(GetWindowSize) {
  NanScope();
  int w,h;
  glfwGetWindowSize(&w, &h);
  Local<Array> arr=Array::New(2);
  arr->Set(JS_STR("width"),JS_INT(w));
  arr->Set(JS_STR("height"),JS_INT(h));
  NanReturnValue(arr);
}

NAN_METHOD(SetWindowSize) {
  NanScope();
  glfwSetWindowSize(args[0]->Uint32Value(),args[1]->Uint32Value());
  NanReturnUndefined();
}

NAN_METHOD(SetWindowPos) {
  NanScope();
  glfwSetWindowPos(args[0]->Uint32Value(),args[1]->Uint32Value());
  NanReturnUndefined();
}

NAN_METHOD(IconifyWindow) {
  NanScope();
  glfwIconifyWindow();
  NanReturnUndefined();
}

NAN_METHOD(RestoreWindow) {
  NanScope();
  glfwRestoreWindow();
  NanReturnUndefined();
}

NAN_METHOD(SwapBuffers) {
  NanScope();
  glfwSwapBuffers();
  NanReturnUndefined();
}

NAN_METHOD(SwapInterval) {
  NanScope();
  glfwSwapInterval(args[0]->Int32Value());
  NanReturnUndefined();
}

NAN_METHOD(GetWindowParam) {
  NanScope();
  NanReturnValue(JS_INT(glfwGetWindowParam(args[0]->Int32Value())));
}
//GLFWAPI void GLFWAPIENTRY glfwSetWindowSizeCallback( GLFWwindowsizefun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetWindowCloseCallback( GLFWwindowclosefun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetWindowRefreshCallback( GLFWwindowrefreshfun cbfun );

/* Input handling */
NAN_METHOD(PollEvents) {
  NanScope();
  glfwPollEvents();
  NanReturnUndefined();
}

NAN_METHOD(WaitEvents) {
  NanScope();
  glfwWaitEvents();
  NanReturnUndefined();
}

NAN_METHOD(GetKey) {
  NanScope();
  NanReturnValue(JS_INT(glfwGetKey(args[0]->Int32Value())));
}

NAN_METHOD(GetMouseButton) {
  NanScope();
  NanReturnValue(JS_INT(glfwGetMouseButton(args[0]->Int32Value())));
}

NAN_METHOD(GetMousePos) {
  NanScope();
  int x,y;
  glfwGetMousePos(&x, &y);
  Local<Array> arr=Array::New(2);
  arr->Set(JS_STR("x"),JS_INT(x));
  arr->Set(JS_STR("y"),JS_INT(y));
  NanReturnValue(arr);
}

NAN_METHOD(SetMousePos) {
  NanScope();
  glfwSetMousePos(args[0]->Int32Value(),args[1]->Int32Value());
  NanReturnUndefined();
}

NAN_METHOD(GetMouseWheel) {
  NanScope();
  NanReturnValue(JS_INT(glfwGetMouseWheel()));
}

NAN_METHOD(SetMouseWheel) {
  NanScope();
  glfwSetMouseWheel(args[0]->Int32Value());
  NanReturnUndefined();
}
//GLFWAPI void GLFWAPIENTRY glfwSetKeyCallback( GLFWkeyfun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetCharCallback( GLFWcharfun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetMouseButtonCallback( GLFWmousebuttonfun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetMousePosCallback( GLFWmouseposfun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetMouseWheelCallback( GLFWmousewheelfun cbfun );

/* Time */
NAN_METHOD(GetTime) {
  NanScope();
  NanReturnValue(JS_NUM(glfwGetTime()));
}

NAN_METHOD(SetTime) {
  NanScope();
  glfwSetTime(args[0]->NumberValue());
  NanReturnUndefined();
}

NAN_METHOD(Sleep) {
  NanScope();
  glfwSleep(args[0]->NumberValue());
  NanReturnUndefined();
}

/* Extension support */
NAN_METHOD(ExtensionSupported) {
  NanScope();
  String::AsciiValue str(args[0]->ToString());
  NanReturnValue(JS_BOOL(glfwExtensionSupported(*str)==1));
}

//GLFWAPI void* GLFWAPIENTRY glfwGetProcAddress( const char *procname );
NAN_METHOD(GetGLVersion) {
  NanScope();
  int major, minor, rev;
  glfwGetGLVersion(&major, &minor, &rev);
  Local<Array> arr=Array::New(3);
  arr->Set(JS_STR("major"),JS_INT(major));
  arr->Set(JS_STR("minor"),JS_INT(minor));
  arr->Set(JS_STR("rev"),JS_INT(rev));
  NanReturnValue(arr);
}

/* Enable/disable functions */
NAN_METHOD(Enable) {
  NanScope();
  glfwEnable(args[0]->Int32Value());
  NanReturnUndefined();
}

NAN_METHOD(Disable) {
  NanScope();
  glfwDisable(args[0]->Int32Value());
  NanReturnUndefined();
}

// make sure we close everything when we exit
void AtExit() {
  TwTerminate();
  glfwTerminate();
}

} // namespace glfw

///////////////////////////////////////////////////////////////////////////////
//
// bindings
//
///////////////////////////////////////////////////////////////////////////////
#define JS_GLFW_CONSTANT(name) target->Set(JS_STR( #name ), JS_INT(GLFW_ ## name))
#define JS_GLFW_SET_METHOD(name) NODE_SET_METHOD(target, #name , glfw::name);

extern "C" {
void init(Handle<Object> target) {
  atexit(glfw::AtExit);

  NanScope();

  /* GLFW initialization, termination and version querying */
  JS_GLFW_SET_METHOD(Init);
  JS_GLFW_SET_METHOD(Terminate);
  JS_GLFW_SET_METHOD(GetVersion);

  /* Window handling */
  JS_GLFW_SET_METHOD(OpenWindow);
  JS_GLFW_SET_METHOD(OpenWindowHint);
  JS_GLFW_SET_METHOD(CloseWindow);
  JS_GLFW_SET_METHOD(SetWindowTitle);
  JS_GLFW_SET_METHOD(GetWindowSize);
  JS_GLFW_SET_METHOD(SetWindowSize);
  JS_GLFW_SET_METHOD(SetWindowPos);
  JS_GLFW_SET_METHOD(IconifyWindow);
  JS_GLFW_SET_METHOD(RestoreWindow);
  JS_GLFW_SET_METHOD(SwapBuffers);
  JS_GLFW_SET_METHOD(SwapInterval);
  JS_GLFW_SET_METHOD(GetWindowParam);

  /* Input handling */
  JS_GLFW_SET_METHOD(PollEvents);
  JS_GLFW_SET_METHOD(WaitEvents);
  JS_GLFW_SET_METHOD(GetKey);
  JS_GLFW_SET_METHOD(GetMouseButton);
  JS_GLFW_SET_METHOD(GetMousePos);
  JS_GLFW_SET_METHOD(SetMousePos);
  JS_GLFW_SET_METHOD(GetMouseWheel);
  JS_GLFW_SET_METHOD(SetMouseWheel);

  /* Time */
  JS_GLFW_SET_METHOD(GetTime);
  JS_GLFW_SET_METHOD(SetTime);
  JS_GLFW_SET_METHOD(Sleep);

  /* Extension support */
  JS_GLFW_SET_METHOD(ExtensionSupported);
  //JS_GLFW_SET_METHOD(GetProcAddress( const char *procname );
  JS_GLFW_SET_METHOD(GetGLVersion);

  /* Enable/disable functions */
  JS_GLFW_SET_METHOD(Enable);
  JS_GLFW_SET_METHOD(Disable);

  /*************************************************************************
   * GLFW version
   *************************************************************************/

  JS_GLFW_CONSTANT(VERSION_MAJOR);
  JS_GLFW_CONSTANT(VERSION_MINOR);
  JS_GLFW_CONSTANT(VERSION_REVISION);


  /*************************************************************************
   * Input handling definitions
   *************************************************************************/

  /* Key and button state/action definitions */
  JS_GLFW_CONSTANT(RELEASE);
  JS_GLFW_CONSTANT(PRESS);

  /* Keyboard key definitions: 8-bit ISO-8859-1 (Latin 1) encoding is used
   * for printable keys (such as A-Z, 0-9 etc), and values above 256
   * represent special (non-printable) keys (e.g. F1, Page Up etc).
   */
  JS_GLFW_CONSTANT(KEY_UNKNOWN);
  JS_GLFW_CONSTANT(KEY_SPACE);
  JS_GLFW_CONSTANT(KEY_SPECIAL);
  JS_GLFW_CONSTANT(KEY_ESC);
  JS_GLFW_CONSTANT(KEY_F1);
  JS_GLFW_CONSTANT(KEY_F2);
  JS_GLFW_CONSTANT(KEY_F3);
  JS_GLFW_CONSTANT(KEY_F4);
  JS_GLFW_CONSTANT(KEY_F5);
  JS_GLFW_CONSTANT(KEY_F6);
  JS_GLFW_CONSTANT(KEY_F7);
  JS_GLFW_CONSTANT(KEY_F8);
  JS_GLFW_CONSTANT(KEY_F9);
  JS_GLFW_CONSTANT(KEY_F10);
  JS_GLFW_CONSTANT(KEY_F11);
  JS_GLFW_CONSTANT(KEY_F12);
  JS_GLFW_CONSTANT(KEY_F13);
  JS_GLFW_CONSTANT(KEY_F14);
  JS_GLFW_CONSTANT(KEY_F15);
  JS_GLFW_CONSTANT(KEY_F16);
  JS_GLFW_CONSTANT(KEY_F17);
  JS_GLFW_CONSTANT(KEY_F18);
  JS_GLFW_CONSTANT(KEY_F19);
  JS_GLFW_CONSTANT(KEY_F20);
  JS_GLFW_CONSTANT(KEY_F21);
  JS_GLFW_CONSTANT(KEY_F22);
  JS_GLFW_CONSTANT(KEY_F23);
  JS_GLFW_CONSTANT(KEY_F24);
  JS_GLFW_CONSTANT(KEY_F25);
  JS_GLFW_CONSTANT(KEY_UP);
  JS_GLFW_CONSTANT(KEY_DOWN);
  JS_GLFW_CONSTANT(KEY_LEFT);
  JS_GLFW_CONSTANT(KEY_RIGHT);
  JS_GLFW_CONSTANT(KEY_LSHIFT);
  JS_GLFW_CONSTANT(KEY_RSHIFT);
  JS_GLFW_CONSTANT(KEY_LCTRL);
  JS_GLFW_CONSTANT(KEY_RCTRL);
  JS_GLFW_CONSTANT(KEY_LALT);
  JS_GLFW_CONSTANT(KEY_RALT);
  JS_GLFW_CONSTANT(KEY_TAB);
  JS_GLFW_CONSTANT(KEY_ENTER);
  JS_GLFW_CONSTANT(KEY_BACKSPACE);
  JS_GLFW_CONSTANT(KEY_INSERT);
  JS_GLFW_CONSTANT(KEY_DEL);
  JS_GLFW_CONSTANT(KEY_PAGEUP);
  JS_GLFW_CONSTANT(KEY_PAGEDOWN);
  JS_GLFW_CONSTANT(KEY_HOME);
  JS_GLFW_CONSTANT(KEY_END);
  JS_GLFW_CONSTANT(KEY_KP_0);
  JS_GLFW_CONSTANT(KEY_KP_1);
  JS_GLFW_CONSTANT(KEY_KP_2);
  JS_GLFW_CONSTANT(KEY_KP_3);
  JS_GLFW_CONSTANT(KEY_KP_4);
  JS_GLFW_CONSTANT(KEY_KP_5);
  JS_GLFW_CONSTANT(KEY_KP_6);
  JS_GLFW_CONSTANT(KEY_KP_7);
  JS_GLFW_CONSTANT(KEY_KP_8);
  JS_GLFW_CONSTANT(KEY_KP_9);
  JS_GLFW_CONSTANT(KEY_KP_DIVIDE);
  JS_GLFW_CONSTANT(KEY_KP_MULTIPLY);
  JS_GLFW_CONSTANT(KEY_KP_SUBTRACT);
  JS_GLFW_CONSTANT(KEY_KP_ADD);
  JS_GLFW_CONSTANT(KEY_KP_DECIMAL);
  JS_GLFW_CONSTANT(KEY_KP_EQUAL);
  JS_GLFW_CONSTANT(KEY_KP_ENTER);
  JS_GLFW_CONSTANT(KEY_KP_NUM_LOCK);
  JS_GLFW_CONSTANT(KEY_CAPS_LOCK);
  JS_GLFW_CONSTANT(KEY_SCROLL_LOCK);
  JS_GLFW_CONSTANT(KEY_PAUSE);
  JS_GLFW_CONSTANT(KEY_LSUPER);
  JS_GLFW_CONSTANT(KEY_RSUPER);
  JS_GLFW_CONSTANT(KEY_MENU);
  JS_GLFW_CONSTANT(KEY_LAST);

  /* Mouse button definitions */
  JS_GLFW_CONSTANT(MOUSE_BUTTON_1);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_2);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_3);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_4);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_5);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_6);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_7);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_8);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_LAST);

  /* Mouse button aliases */
  JS_GLFW_CONSTANT(MOUSE_BUTTON_LEFT);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_RIGHT);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_MIDDLE);


  /* Joystick identifiers */
  JS_GLFW_CONSTANT(JOYSTICK_1);
  JS_GLFW_CONSTANT(JOYSTICK_2);
  JS_GLFW_CONSTANT(JOYSTICK_3);
  JS_GLFW_CONSTANT(JOYSTICK_4);
  JS_GLFW_CONSTANT(JOYSTICK_5);
  JS_GLFW_CONSTANT(JOYSTICK_6);
  JS_GLFW_CONSTANT(JOYSTICK_7);
  JS_GLFW_CONSTANT(JOYSTICK_8);
  JS_GLFW_CONSTANT(JOYSTICK_9);
  JS_GLFW_CONSTANT(JOYSTICK_10);
  JS_GLFW_CONSTANT(JOYSTICK_11);
  JS_GLFW_CONSTANT(JOYSTICK_12);
  JS_GLFW_CONSTANT(JOYSTICK_13);
  JS_GLFW_CONSTANT(JOYSTICK_14);
  JS_GLFW_CONSTANT(JOYSTICK_15);
  JS_GLFW_CONSTANT(JOYSTICK_16);
  JS_GLFW_CONSTANT(JOYSTICK_LAST);


  /*************************************************************************
   * Other definitions
   *************************************************************************/

  /* glfwOpenWindow modes */
  JS_GLFW_CONSTANT(WINDOW);
  JS_GLFW_CONSTANT(FULLSCREEN);

  /* glfwGetWindowParam tokens */
  JS_GLFW_CONSTANT(OPENED);
  JS_GLFW_CONSTANT(ACTIVE);
  JS_GLFW_CONSTANT(ICONIFIED);
  JS_GLFW_CONSTANT(ACCELERATED);
  JS_GLFW_CONSTANT(RED_BITS);
  JS_GLFW_CONSTANT(GREEN_BITS);
  JS_GLFW_CONSTANT(BLUE_BITS);
  JS_GLFW_CONSTANT(ALPHA_BITS);
  JS_GLFW_CONSTANT(DEPTH_BITS);
  JS_GLFW_CONSTANT(STENCIL_BITS);

  /* The following constants are used for both glfwGetWindowParam
   * and glfwOpenWindowHint
   */
  JS_GLFW_CONSTANT(REFRESH_RATE);
  JS_GLFW_CONSTANT(ACCUM_RED_BITS);
  JS_GLFW_CONSTANT(ACCUM_GREEN_BITS);
  JS_GLFW_CONSTANT(ACCUM_BLUE_BITS);
  JS_GLFW_CONSTANT(ACCUM_ALPHA_BITS);
  JS_GLFW_CONSTANT(AUX_BUFFERS);
  JS_GLFW_CONSTANT(STEREO);
  JS_GLFW_CONSTANT(WINDOW_NO_RESIZE);
  JS_GLFW_CONSTANT(FSAA_SAMPLES);
  JS_GLFW_CONSTANT(OPENGL_VERSION_MAJOR);
  JS_GLFW_CONSTANT(OPENGL_VERSION_MINOR);
  JS_GLFW_CONSTANT(OPENGL_FORWARD_COMPAT);
  JS_GLFW_CONSTANT(OPENGL_DEBUG_CONTEXT);
  JS_GLFW_CONSTANT(OPENGL_PROFILE);

  /* GLFW_OPENGL_PROFILE tokens */
  JS_GLFW_CONSTANT(OPENGL_CORE_PROFILE);
  JS_GLFW_CONSTANT(OPENGL_COMPAT_PROFILE);

  /* glfwEnable/glfwDisable tokens */
  JS_GLFW_CONSTANT(MOUSE_CURSOR);
  JS_GLFW_CONSTANT(STICKY_KEYS);
  JS_GLFW_CONSTANT(STICKY_MOUSE_BUTTONS);
  JS_GLFW_CONSTANT(SYSTEM_KEYS);
  JS_GLFW_CONSTANT(KEY_REPEAT);
  JS_GLFW_CONSTANT(AUTO_POLL_EVENTS);

  /* glfwWaitThread wait modes */
  JS_GLFW_CONSTANT(WAIT);
  JS_GLFW_CONSTANT(NOWAIT);

  /* glfwGetJoystickParam tokens */
  JS_GLFW_CONSTANT(PRESENT);
  JS_GLFW_CONSTANT(AXES);
  JS_GLFW_CONSTANT(BUTTONS);

  /* glfwReadImage/glfwLoadTexture2D flags */
  JS_GLFW_CONSTANT(NO_RESCALE_BIT); /* Only for glfwReadImage */
  JS_GLFW_CONSTANT(ORIGIN_UL_BIT);
  JS_GLFW_CONSTANT(BUILD_MIPMAPS_BIT); /* Only for glfwLoadTexture2D */
  JS_GLFW_CONSTANT(ALPHA_MAP_BIT);

  /* Time spans longer than this (seconds) are considered to be infinity */
  JS_GLFW_CONSTANT(INFINITY);

  // init AntTweakBar
  atb::AntTweakBar::Initialize(target);
  atb::Bar::Initialize(target);

  // test scene
  JS_GLFW_SET_METHOD(testScene);
}

NODE_MODULE(glfw, init)
}

