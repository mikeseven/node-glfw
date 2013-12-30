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

/* @Module: initialization and version information */

JS_METHOD(Init) {
  HandleScope scope;
  return scope.Close(JS_BOOL(glfwInit()==GL_TRUE));
}

JS_METHOD(Terminate) {
  HandleScope scope;
  glfwTerminate();
  return scope.Close(Undefined());
}

JS_METHOD(GetVersion) {
  HandleScope scope;
  int major, minor, rev;
  glfwGetVersion(&major,&minor,&rev);
  Local<Array> arr=Array::New(3);
  arr->Set(JS_STR("major"),JS_INT(major));
  arr->Set(JS_STR("minor"),JS_INT(minor));
  arr->Set(JS_STR("rev"),JS_INT(rev));
  return scope.Close(arr);
}

JS_METHOD(GetVersionString) {
  HandleScope scope;
  const char* ver=glfwGetVersionString();
  return scope.Close(JS_STR(ver));
}

/* @Module: Time input */

JS_METHOD(GetTime) {
  HandleScope scope;
  return scope.Close(JS_NUM(glfwGetTime()));
}

JS_METHOD(SetTime) {
  HandleScope scope;
  double time = args[0]->NumberValue();
  glfwSetTime(time);
  return scope.Close(Undefined());
}

/* @Module: monitor handling */

/* @Module: Window handling */
Persistent<Object> glfw_events;
int lastX=0,lastY=0;
bool windowCreated=false;

/* Window callbacks handling */
void APIENTRY windowPosCB(GLFWwindow *window, int xpos, int ypos) {
  HandleScope scope;
  //cout<<"resizeCB: "<<w<<" "<<h<<endl;

  Local<Array> evt=Array::New(3);
  evt->Set(JS_STR("type"),JS_STR("window_pos"));
  evt->Set(JS_STR("xpos"),JS_INT(xpos));
  evt->Set(JS_STR("ypos"),JS_INT(ypos));

  Handle<Value> argv[2] = {
    JS_STR("window_pos"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

void APIENTRY windowSizeCB(GLFWwindow *window, int w, int h) {
  HandleScope scope;
  //cout<<"resizeCB: "<<w<<" "<<h<<endl;

  Local<Array> evt=Array::New(3);
  evt->Set(JS_STR("type"),JS_STR("resize"));
  evt->Set(JS_STR("width"),JS_INT(w));
  evt->Set(JS_STR("height"),JS_INT(h));

  Handle<Value> argv[2] = {
    JS_STR("resize"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

void APIENTRY windowFramebufferSizeCB(GLFWwindow *window, int w, int h) {
  HandleScope scope;
  //cout<<"resizeCB: "<<w<<" "<<h<<endl;

  Local<Array> evt=Array::New(3);
  evt->Set(JS_STR("type"),JS_STR("framebuffer_resize"));
  evt->Set(JS_STR("width"),JS_INT(w));
  evt->Set(JS_STR("height"),JS_INT(h));

  Handle<Value> argv[2] = {
    JS_STR("framebuffer_resize"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

void APIENTRY windowCloseCB(GLFWwindow *window) {
  HandleScope scope;

  Handle<Value> argv[1] = {
    JS_STR("quit"), // event name
  };

  MakeCallback(glfw_events, "emit", 1, argv);
}

void APIENTRY windowRefreshCB(GLFWwindow *window) {
  HandleScope scope;

  Local<Array> evt=Array::New(2);
  evt->Set(JS_STR("type"),JS_STR("refresh"));
  evt->Set(JS_STR("window"),JS_NUM((uint64_t) window));

  Handle<Value> argv[2] = {
    JS_STR("refresh"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

void APIENTRY windowIconifyCB(GLFWwindow *window, int iconified) {
  HandleScope scope;

  Local<Array> evt=Array::New(2);
  evt->Set(JS_STR("type"),JS_STR("iconified"));
  evt->Set(JS_STR("iconified"),JS_BOOL(iconified));

  Handle<Value> argv[2] = {
    JS_STR("iconified"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

void APIENTRY windowFocusCB(GLFWwindow *window, int focused) {
  HandleScope scope;

  Local<Array> evt=Array::New(2);
  evt->Set(JS_STR("type"),JS_STR("focused"));
  evt->Set(JS_STR("focused"),JS_BOOL(focused));

  Handle<Value> argv[2] = {
    JS_STR("focused"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

/* Input callbacks handling */
void APIENTRY keyCB(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if(!TwEventKeyGLFW(key,action)) {
    HandleScope scope;

    Local<Array> evt=Array::New(7);
    evt->Set(JS_STR("type"),JS_STR(action ? "keydown" : "keyup"));
    evt->Set(JS_STR("ctrlKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL)));
    evt->Set(JS_STR("shiftKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT)));
    evt->Set(JS_STR("altKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_ALT) || glfwGetKey(window, GLFW_KEY_RIGHT_ALT)));
    evt->Set(JS_STR("metaKey"),JS_BOOL(glfwGetKey(window, GLFW_KEY_LEFT_SUPER) || glfwGetKey(window, GLFW_KEY_RIGHT_SUPER)));

    if(key==GLFW_KEY_ESCAPE) key=27;
    else if(key==GLFW_KEY_LEFT_SHIFT || key==GLFW_KEY_RIGHT_SHIFT) key=16;
    else if(key==GLFW_KEY_LEFT_CONTROL || key==GLFW_KEY_RIGHT_CONTROL) key=17;
    else if(key==GLFW_KEY_LEFT_ALT || key==GLFW_KEY_RIGHT_ALT) key=18;
    else if(key==GLFW_KEY_LEFT_SUPER) key=91;
    else if(key==GLFW_KEY_RIGHT_SUPER) key=93;
    evt->Set(JS_STR("which"),JS_INT(key));
    evt->Set(JS_STR("keyCode"),JS_INT(key));
    evt->Set(JS_STR("charCode"),JS_INT(key));

    Handle<Value> argv[2] = {
      JS_STR(action ? "keydown" : "keyup"), // event name
      evt
    };

    MakeCallback(glfw_events, "emit", 2, argv);
  }
}

void APIENTRY cursorPosCB(GLFWwindow* window, double x, double y) {
  if(!TwEventMousePosGLFW(x,y)) {
    int w,h;
    glfwGetWindowSize(window, &w, &h);
    if(x<0 || x>=w) return;
    if(y<0 || y>=h) return;

    lastX=x;
    lastY=y;

    HandleScope scope;

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

    MakeCallback(glfw_events, "emit", 2, argv);
  }
}

void APIENTRY cursorEnterCB(GLFWwindow* window, int entered) {
  HandleScope scope;

  Local<Array> evt=Array::New(2);
  evt->Set(JS_STR("type"),JS_STR("mouseenter"));
  evt->Set(JS_STR("entered"),JS_INT(entered));

  Handle<Value> argv[2] = {
    JS_STR("mouseenter"), // event name
    evt
  };

  MakeCallback(glfw_events, "emit", 2, argv);
}

void APIENTRY mouseButtonCB(GLFWwindow *window, int button, int action, int mods) {
  if(!TwEventMouseButtonGLFW(button,action)) {
    HandleScope scope;
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

    MakeCallback(glfw_events, "emit", 2, argv);
  }
}

void APIENTRY scrollCB(GLFWwindow *window, double xoffset, double yoffset) {
  if(!TwEventMouseWheelGLFW(yoffset)) {
    HandleScope scope;

    Local<Array> evt=Array::New(3);
    evt->Set(JS_STR("type"),JS_STR("scroll"));
    evt->Set(JS_STR("xoffset"),JS_INT(xoffset));
    evt->Set(JS_STR("yoffset"),JS_INT(yoffset));

    Handle<Value> argv[2] = {
      JS_STR("scroll"), // event name
      evt
    };

    MakeCallback(glfw_events, "emit", 2, argv);
  }
}

JS_METHOD(testScene) {
  HandleScope scope;
  glViewport( 0, 0, args[0]->Uint32Value(), args[1]->Uint32Value() );

  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
  glClear( GL_COLOR_BUFFER_BIT );
  
  glPushMatrix();
  //glRotatef( 0, 0.0f, 0.0f, 1.0f );
  glBegin( GL_TRIANGLES );
    glColor3f( 1.0f, 0.0f, 0.0f ); glVertex2f( 0.0f, 1.0f );
    glColor3f( 0.0f, 1.0f, 0.0f ); glVertex2f( 0.87f, -0.5f );
    glColor3f( 0.0f, 0.0f, 1.0f ); glVertex2f( -0.87f, -0.5f );
  glEnd();
  glPopMatrix();

  return scope.Close(Undefined());
}

JS_METHOD(WindowHint) {
  HandleScope scope;
  int target       = args[0]->Uint32Value();
  int hint         = args[1]->Uint32Value();
  glfwWindowHint(target, hint);
  return scope.Close(Undefined());
}

JS_METHOD(DefaultWindowHints) {
  HandleScope scope;
  glfwDefaultWindowHints();
  return scope.Close(Undefined());
}

JS_METHOD(CreateWindow) {
  HandleScope scope;
  int width       = args[0]->Uint32Value();
  int height      = args[1]->Uint32Value();
  String::Utf8Value str(args[2]->ToString());
  GLFWwindow* window = NULL;

  if(!windowCreated) {
    window = glfwCreateWindow(width, height, *str, NULL, NULL);
    if(!window) {
      // can't create window, throw error
      return ThrowError("Can't create GLFW window");
    }

    glfwMakeContextCurrent(window);

    GLenum err = glewInit();
    if (err)
    {
      /* Problem: glewInit failed, something is seriously wrong. */
      string msg="Can't init GLEW (glew error ";
      msg+=(const char*) glewGetErrorString(err);
      msg+=")";

      fprintf(stderr, "%s", msg.c_str());
      return ThrowError(msg.c_str());
    }
    fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
  }
  else
    glfwSetWindowSize(window, width,height);

  // Set callback functions
  glfw_events=Persistent<Object>::New(args.This()->Get(JS_STR("events"))->ToObject());

  // window callbacks
  glfwSetWindowPosCallback( window, windowPosCB );
  glfwSetWindowSizeCallback( window, windowSizeCB );
  glfwSetWindowCloseCallback( window, windowCloseCB );
  glfwSetWindowRefreshCallback( window, windowRefreshCB );
  glfwSetWindowFocusCallback( window, windowFocusCB );
  glfwSetWindowIconifyCallback( window, windowIconifyCB );
  glfwSetFramebufferSizeCallback( window, windowFramebufferSizeCB );

  // input callbacks
  glfwSetKeyCallback( window, keyCB);
  // TODO glfwSetCharCallback(window, TwEventCharGLFW);
  glfwSetMouseButtonCallback( window, mouseButtonCB );
  glfwSetCursorPosCallback( window, cursorPosCB );
  glfwSetCursorEnterCallback( window, cursorEnterCB );
  glfwSetScrollCallback( window, scrollCB );

  return scope.Close(JS_NUM((uint64_t) window));
}

JS_METHOD(DestroyWindow) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwDestroyWindow(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(SetWindowTitle) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  String::Utf8Value str(args[1]->ToString());
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwSetWindowTitle(window, *str);
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetWindowSize) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    int w,h;
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwGetWindowSize(window, &w, &h);
    Local<Array> arr=Array::New(2);
    arr->Set(JS_STR("width"),JS_INT(w));
    arr->Set(JS_STR("height"),JS_INT(h));
    return scope.Close(arr);
  }
  return scope.Close(Undefined());
}

JS_METHOD(SetWindowSize) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwSetWindowSize(window, args[1]->Uint32Value(),args[2]->Uint32Value());
  }
  return scope.Close(Undefined());
}

JS_METHOD(SetWindowPos) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwSetWindowPos(window, args[1]->Uint32Value(),args[2]->Uint32Value());
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetWindowPos) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    int xpos, ypos;
    glfwGetWindowPos(window, &xpos, &ypos);
    Local<Array> arr=Array::New(2);
    arr->Set(JS_STR("xpos"),JS_INT(xpos));
    arr->Set(JS_STR("ypos"),JS_INT(ypos));
    return scope.Close(arr);
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetFramebufferSize) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    int width, height;
    glfwGetWindowPos(window, &width, &height);
    Local<Array> arr=Array::New(2);
    arr->Set(JS_STR("width"),JS_INT(width));
    arr->Set(JS_STR("height"),JS_INT(height));
    return scope.Close(arr);
  }
  return scope.Close(Undefined());
}

JS_METHOD(IconifyWindow) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwIconifyWindow(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(RestoreWindow) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwRestoreWindow(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(HideWindow) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwHideWindow(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(ShowWindow) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwShowWindow(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(WindowShouldClose) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    return scope.Close(JS_INT(glfwWindowShouldClose(window)));
  }
  return scope.Close(Undefined());
}

JS_METHOD(SetWindowShouldClose) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  int value=args[1]->Uint32Value();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwSetWindowShouldClose(window, value);
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetWindowAttrib) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  int attrib=args[1]->Uint32Value();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    return scope.Close(JS_INT(glfwGetWindowAttrib(window, attrib)));
  }
  return scope.Close(Undefined());
}

JS_METHOD(PollEvents) {
  HandleScope scope;
  glfwPollEvents();
  return scope.Close(Undefined());
}

JS_METHOD(WaitEvents) {
  HandleScope scope;
  glfwWaitEvents();
  return scope.Close(Undefined());
}

//GLFWAPI void GLFWAPIENTRY glfwSetWindowSizeCallback( GLFWwindowsizefun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetWindowCloseCallback( GLFWwindowclosefun cbfun );
//GLFWAPI void GLFWAPIENTRY glfwSetWindowRefreshCallback( GLFWwindowrefreshfun cbfun );

/* Input handling */

JS_METHOD(GetKey) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  int key=args[1]->Uint32Value();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    return scope.Close(JS_INT(glfwGetKey(window, key)));
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetMouseButton) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  int button=args[1]->Uint32Value();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    return scope.Close(JS_INT(glfwGetMouseButton(window, button)));
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetCursorPos) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    double x,y;
    glfwGetCursorPos(window, &x, &y);
    Local<Array> arr=Array::New(2);
    arr->Set(JS_STR("x"),JS_INT(x));
    arr->Set(JS_STR("y"),JS_INT(y));
    return scope.Close(arr);
  }
  return scope.Close(Undefined());
}

JS_METHOD(SetCursorPos) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  int x=args[1]->NumberValue();
  int y=args[2]->NumberValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwSetCursorPos(window, x, y);
  }
  return scope.Close(Undefined());
}

/* @Module Context handling */
JS_METHOD(MakeContextCurrent) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwMakeContextCurrent(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(GetCurrentContext) {
  HandleScope scope;
  GLFWwindow* window=glfwGetCurrentContext();
  return scope.Close(JS_NUM((uint64_t) window));
}

JS_METHOD(SwapBuffers) {
  HandleScope scope;
  uint64_t handle=args[0]->IntegerValue();
  if(handle) {
    GLFWwindow* window = reinterpret_cast<GLFWwindow*>(handle);
    glfwSwapBuffers(window);
  }
  return scope.Close(Undefined());
}

JS_METHOD(SwapInterval) {
  HandleScope scope;
  int interval=args[0]->Int32Value();
  glfwSwapInterval(interval);
  return scope.Close(Undefined());
}

JS_METHOD(ExtensionSupported) {
  HandleScope scope;
  String::AsciiValue str(args[0]->ToString());
  return scope.Close(JS_BOOL(glfwExtensionSupported(*str)==1));
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

  HandleScope scope;

  /* GLFW initialization, termination and version querying */
  JS_GLFW_SET_METHOD(Init);
  JS_GLFW_SET_METHOD(Terminate);
  JS_GLFW_SET_METHOD(GetVersion);
  JS_GLFW_SET_METHOD(GetVersionString);

  /* Time */
  JS_GLFW_SET_METHOD(GetTime);
  JS_GLFW_SET_METHOD(SetTime);

  /* Window handling */
  JS_GLFW_SET_METHOD(CreateWindow);
  JS_GLFW_SET_METHOD(WindowHint);
  JS_GLFW_SET_METHOD(DestroyWindow);
  JS_GLFW_SET_METHOD(SetWindowShouldClose);
  JS_GLFW_SET_METHOD(WindowShouldClose);
  JS_GLFW_SET_METHOD(SetWindowTitle);
  JS_GLFW_SET_METHOD(GetWindowSize);
  JS_GLFW_SET_METHOD(SetWindowSize);
  JS_GLFW_SET_METHOD(SetWindowPos);
  JS_GLFW_SET_METHOD(GetWindowPos);
  JS_GLFW_SET_METHOD(GetFramebufferSize);
  JS_GLFW_SET_METHOD(IconifyWindow);
  JS_GLFW_SET_METHOD(RestoreWindow);
  JS_GLFW_SET_METHOD(ShowWindow);
  JS_GLFW_SET_METHOD(HideWindow);
  JS_GLFW_SET_METHOD(GetWindowAttrib);
  JS_GLFW_SET_METHOD(PollEvents);
  JS_GLFW_SET_METHOD(WaitEvents);

  /* Input handling */
  JS_GLFW_SET_METHOD(GetKey);
  JS_GLFW_SET_METHOD(GetMouseButton);
  JS_GLFW_SET_METHOD(GetCursorPos);
  JS_GLFW_SET_METHOD(SetCursorPos);

  /* Context handling */
  JS_GLFW_SET_METHOD(MakeContextCurrent);
  JS_GLFW_SET_METHOD(GetCurrentContext);
  JS_GLFW_SET_METHOD(SwapBuffers);
  JS_GLFW_SET_METHOD(SwapInterval);
  JS_GLFW_SET_METHOD(ExtensionSupported);

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
  JS_GLFW_CONSTANT(REPEAT);

  /* These key codes are inspired by the *USB HID Usage Tables v1.12* (p. 53-60),
   * but re-arranged to map to 7-bit ASCII for printable keys (function keys are
   * put in the 256+ range).
   *
   * The naming of the key codes follow these rules:
   *  - The US keyboard layout is used
   *  - Names of printable alpha-numeric characters are used (e.g. "A", "R",
   *    "3", etc.)
   *  - For non-alphanumeric characters, Unicode:ish names are used (e.g.
   *    "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
   *    correspond to the Unicode standard (usually for brevity)
   *  - Keys that lack a clear US mapping are named "WORLD_x"
   *  - For non-printable keys, custom names are used (e.g. "F4",
   *    "BACKSPACE", etc.)
   */

  /* The unknown key */
  JS_GLFW_CONSTANT(KEY_UNKNOWN);

  /* Printable keys */
  JS_GLFW_CONSTANT(KEY_SPACE);
  JS_GLFW_CONSTANT(KEY_APOSTROPHE);
  JS_GLFW_CONSTANT(KEY_COMMA);
  JS_GLFW_CONSTANT(KEY_MINUS);
  JS_GLFW_CONSTANT(KEY_PERIOD);
  JS_GLFW_CONSTANT(KEY_SLASH);
  JS_GLFW_CONSTANT(KEY_0);
  JS_GLFW_CONSTANT(KEY_1);
  JS_GLFW_CONSTANT(KEY_2);
  JS_GLFW_CONSTANT(KEY_3);
  JS_GLFW_CONSTANT(KEY_4);
  JS_GLFW_CONSTANT(KEY_5);
  JS_GLFW_CONSTANT(KEY_6);
  JS_GLFW_CONSTANT(KEY_7);
  JS_GLFW_CONSTANT(KEY_8);
  JS_GLFW_CONSTANT(KEY_9);
  JS_GLFW_CONSTANT(KEY_SEMICOLON);
  JS_GLFW_CONSTANT(KEY_EQUAL);
  JS_GLFW_CONSTANT(KEY_A);
  JS_GLFW_CONSTANT(KEY_B);
  JS_GLFW_CONSTANT(KEY_C);
  JS_GLFW_CONSTANT(KEY_D);
  JS_GLFW_CONSTANT(KEY_E);
  JS_GLFW_CONSTANT(KEY_F);
  JS_GLFW_CONSTANT(KEY_G);
  JS_GLFW_CONSTANT(KEY_H);
  JS_GLFW_CONSTANT(KEY_I);
  JS_GLFW_CONSTANT(KEY_J);
  JS_GLFW_CONSTANT(KEY_K);
  JS_GLFW_CONSTANT(KEY_L);
  JS_GLFW_CONSTANT(KEY_M);
  JS_GLFW_CONSTANT(KEY_N);
  JS_GLFW_CONSTANT(KEY_O);
  JS_GLFW_CONSTANT(KEY_P);
  JS_GLFW_CONSTANT(KEY_Q);
  JS_GLFW_CONSTANT(KEY_R);
  JS_GLFW_CONSTANT(KEY_S);
  JS_GLFW_CONSTANT(KEY_T);
  JS_GLFW_CONSTANT(KEY_U);
  JS_GLFW_CONSTANT(KEY_V);
  JS_GLFW_CONSTANT(KEY_W);
  JS_GLFW_CONSTANT(KEY_X);
  JS_GLFW_CONSTANT(KEY_Y);
  JS_GLFW_CONSTANT(KEY_Z);
  JS_GLFW_CONSTANT(KEY_LEFT_BRACKET);
  JS_GLFW_CONSTANT(KEY_BACKSLASH);
  JS_GLFW_CONSTANT(KEY_RIGHT_BRACKET);
  JS_GLFW_CONSTANT(KEY_GRAVE_ACCENT);
  JS_GLFW_CONSTANT(KEY_WORLD_1);
  JS_GLFW_CONSTANT(KEY_WORLD_2);

  /* Function keys */
  JS_GLFW_CONSTANT(KEY_ESCAPE);
  JS_GLFW_CONSTANT(KEY_ENTER);
  JS_GLFW_CONSTANT(KEY_TAB);
  JS_GLFW_CONSTANT(KEY_BACKSPACE);
  JS_GLFW_CONSTANT(KEY_INSERT);
  JS_GLFW_CONSTANT(KEY_DELETE);
  JS_GLFW_CONSTANT(KEY_RIGHT);
  JS_GLFW_CONSTANT(KEY_LEFT);
  JS_GLFW_CONSTANT(KEY_DOWN);
  JS_GLFW_CONSTANT(KEY_UP);
  JS_GLFW_CONSTANT(KEY_PAGE_UP);
  JS_GLFW_CONSTANT(KEY_PAGE_DOWN);
  JS_GLFW_CONSTANT(KEY_HOME);
  JS_GLFW_CONSTANT(KEY_END);
  JS_GLFW_CONSTANT(KEY_CAPS_LOCK);
  JS_GLFW_CONSTANT(KEY_SCROLL_LOCK);
  JS_GLFW_CONSTANT(KEY_NUM_LOCK);
  JS_GLFW_CONSTANT(KEY_PRINT_SCREEN);
  JS_GLFW_CONSTANT(KEY_PAUSE);
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
  JS_GLFW_CONSTANT(KEY_KP_DECIMAL);
  JS_GLFW_CONSTANT(KEY_KP_DIVIDE);
  JS_GLFW_CONSTANT(KEY_KP_MULTIPLY);
  JS_GLFW_CONSTANT(KEY_KP_SUBTRACT);
  JS_GLFW_CONSTANT(KEY_KP_ADD);
  JS_GLFW_CONSTANT(KEY_KP_ENTER);
  JS_GLFW_CONSTANT(KEY_KP_EQUAL);
  JS_GLFW_CONSTANT(KEY_LEFT_SHIFT);
  JS_GLFW_CONSTANT(KEY_LEFT_CONTROL);
  JS_GLFW_CONSTANT(KEY_LEFT_ALT);
  JS_GLFW_CONSTANT(KEY_LEFT_SUPER);
  JS_GLFW_CONSTANT(KEY_RIGHT_SHIFT);
  JS_GLFW_CONSTANT(KEY_RIGHT_CONTROL);
  JS_GLFW_CONSTANT(KEY_RIGHT_ALT);
  JS_GLFW_CONSTANT(KEY_RIGHT_SUPER);
  JS_GLFW_CONSTANT(KEY_MENU);
  JS_GLFW_CONSTANT(KEY_LAST);

  /*Modifier key flags*/

  /*If this bit is set one or more Shift keys were held down. */
  JS_GLFW_CONSTANT(MOD_SHIFT);
  /*If this bit is set one or more Control keys were held down. */
  JS_GLFW_CONSTANT(MOD_CONTROL);
  /*If this bit is set one or more Alt keys were held down. */
  JS_GLFW_CONSTANT(MOD_ALT);
  /*If this bit is set one or more Super keys were held down. */
  JS_GLFW_CONSTANT(MOD_SUPER);

  /*Mouse buttons*/
  JS_GLFW_CONSTANT(MOUSE_BUTTON_1);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_2);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_3);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_4);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_5);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_6);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_7);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_8);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_LAST);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_LEFT);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_RIGHT);
  JS_GLFW_CONSTANT(MOUSE_BUTTON_MIDDLE);

  /*Joysticks*/
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

  /*errors Error codes*/

  /*GLFW has not been initialized.*/
  JS_GLFW_CONSTANT(NOT_INITIALIZED);
  /*No context is current for this thread.*/
  JS_GLFW_CONSTANT(NO_CURRENT_CONTEXT);
  /*One of the enum parameters for the function was given an invalid enum.*/
  JS_GLFW_CONSTANT(INVALID_ENUM);
  /*One of the parameters for the function was given an invalid value.*/
  JS_GLFW_CONSTANT(INVALID_VALUE);
  /*A memory allocation failed.*/
  JS_GLFW_CONSTANT(OUT_OF_MEMORY);
  /*GLFW could not find support for the requested client API on the system.*/
  JS_GLFW_CONSTANT(API_UNAVAILABLE);
  /*The requested client API version is not available.*/
  JS_GLFW_CONSTANT(VERSION_UNAVAILABLE);
  /*A platform-specific error occurred that does not match any of the more specific categories.*/
  JS_GLFW_CONSTANT(PLATFORM_ERROR);
  /*The clipboard did not contain data in the requested format.*/
  JS_GLFW_CONSTANT(FORMAT_UNAVAILABLE);

  JS_GLFW_CONSTANT(FOCUSED);
  JS_GLFW_CONSTANT(ICONIFIED);
  JS_GLFW_CONSTANT(RESIZABLE);
  JS_GLFW_CONSTANT(VISIBLE);
  JS_GLFW_CONSTANT(DECORATED);

  JS_GLFW_CONSTANT(RED_BITS);
  JS_GLFW_CONSTANT(GREEN_BITS);
  JS_GLFW_CONSTANT(BLUE_BITS);
  JS_GLFW_CONSTANT(ALPHA_BITS);
  JS_GLFW_CONSTANT(DEPTH_BITS);
  JS_GLFW_CONSTANT(STENCIL_BITS);
  JS_GLFW_CONSTANT(ACCUM_RED_BITS);
  JS_GLFW_CONSTANT(ACCUM_GREEN_BITS);
  JS_GLFW_CONSTANT(ACCUM_BLUE_BITS);
  JS_GLFW_CONSTANT(ACCUM_ALPHA_BITS);
  JS_GLFW_CONSTANT(AUX_BUFFERS);
  JS_GLFW_CONSTANT(STEREO);
  JS_GLFW_CONSTANT(SAMPLES);
  JS_GLFW_CONSTANT(SRGB_CAPABLE);
  JS_GLFW_CONSTANT(REFRESH_RATE);

  JS_GLFW_CONSTANT(CLIENT_API);
  JS_GLFW_CONSTANT(CONTEXT_VERSION_MAJOR);
  JS_GLFW_CONSTANT(CONTEXT_VERSION_MINOR);
  JS_GLFW_CONSTANT(CONTEXT_REVISION);
  JS_GLFW_CONSTANT(CONTEXT_ROBUSTNESS);
  JS_GLFW_CONSTANT(OPENGL_FORWARD_COMPAT);
  JS_GLFW_CONSTANT(OPENGL_DEBUG_CONTEXT);
  JS_GLFW_CONSTANT(OPENGL_PROFILE);

  JS_GLFW_CONSTANT(OPENGL_API);
  JS_GLFW_CONSTANT(OPENGL_ES_API);

  JS_GLFW_CONSTANT(NO_ROBUSTNESS);
  JS_GLFW_CONSTANT(NO_RESET_NOTIFICATION);
  JS_GLFW_CONSTANT(LOSE_CONTEXT_ON_RESET);

  JS_GLFW_CONSTANT(OPENGL_ANY_PROFILE);
  JS_GLFW_CONSTANT(OPENGL_CORE_PROFILE);
  JS_GLFW_CONSTANT(OPENGL_COMPAT_PROFILE);

  JS_GLFW_CONSTANT(CURSOR);
  JS_GLFW_CONSTANT(STICKY_KEYS);
  JS_GLFW_CONSTANT(STICKY_MOUSE_BUTTONS);

  JS_GLFW_CONSTANT(CURSOR_NORMAL);
  JS_GLFW_CONSTANT(CURSOR_HIDDEN);
  JS_GLFW_CONSTANT(CURSOR_DISABLED);

  JS_GLFW_CONSTANT(CONNECTED);
  JS_GLFW_CONSTANT(DISCONNECTED);

  // init AntTweakBar
  atb::AntTweakBar::Initialize(target);
  atb::Bar::Initialize(target);

  // test scene
  JS_GLFW_SET_METHOD(testScene);
}

NODE_MODULE(glfw, init)
}

