/*
 * atb.cc
 *
 *  Created on: Mar 8, 2012
 *      Author: ngk437
 */

// OpenGL Graphics Includes
#define GLFW_INCLUDE_GL3
#define GLFW_NO_GLU
#include <GL/glfw.h>

#include "atb.h"

#include <iostream>
using namespace std;

namespace atb {
Persistent<FunctionTemplate> AntTweakBar::constructor_template;

#define DEFINE_ATB_CONSTANT(constant) \
    NODE_DEFINE_CONSTANT_VALUE(constructor_template->InstanceTemplate(), "TYPE_" #constant, TW_TYPE_##constant);

void AntTweakBar::Initialize (Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  constructor_template = Persistent<FunctionTemplate>::New(t);

  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(JS_STR("AntTweakBar"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "Init", Init);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "Terminate", Terminate);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "WindowSize", WindowSize);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "Draw", Draw);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "NewBar", NewBar);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "Define", Define);

#define NODE_DEFINE_CONSTANT_VALUE(target, name, value)                   \
  (target)->Set(v8::String::NewSymbol(name),                         \
                v8::Integer::New(value),                               \
                static_cast<v8::PropertyAttribute>(v8::ReadOnly|v8::DontDelete))

  DEFINE_ATB_CONSTANT(CHAR);
  DEFINE_ATB_CONSTANT(INT8);
  DEFINE_ATB_CONSTANT(UINT8);
  DEFINE_ATB_CONSTANT(INT16);
  DEFINE_ATB_CONSTANT(UINT16);
  DEFINE_ATB_CONSTANT(INT32);
  DEFINE_ATB_CONSTANT(UINT32);
  DEFINE_ATB_CONSTANT(FLOAT);
  DEFINE_ATB_CONSTANT(DOUBLE);

  DEFINE_ATB_CONSTANT(COLOR32);
  DEFINE_ATB_CONSTANT(COLOR3F);
  DEFINE_ATB_CONSTANT(COLOR4F);
  DEFINE_ATB_CONSTANT(QUAT4F);
  DEFINE_ATB_CONSTANT(QUAT4D);
  DEFINE_ATB_CONSTANT(DIR3F);
  DEFINE_ATB_CONSTANT(DIR3D);

  //DEFINE_ATB_CONSTANT(CDSTRING);

  target->Set(String::NewSymbol("AntTweakBar"), constructor_template->GetFunction());
}

Handle<Value> AntTweakBar::New (const Arguments& args) {
  if (!args.IsConstructCall())
    return ThrowTypeError("Constructor cannot be called as a function.");

  HandleScope scope;
  AntTweakBar *cl = new AntTweakBar(args.This());
  cl->Wrap(args.This());
  return scope.Close(args.This());
}

AntTweakBar::AntTweakBar(Handle<Object> wrapper)
{
}

AntTweakBar::~AntTweakBar () {
  TwTerminate();
}

JS_METHOD(AntTweakBar::Init) {
  HandleScope scope;
  TwInit(TW_OPENGL, NULL);
  return scope.Close(Undefined());
}

JS_METHOD(AntTweakBar::Terminate) {
  HandleScope scope;
  TwTerminate();
  return scope.Close(Undefined());
}

JS_METHOD(AntTweakBar::WindowSize) {
  HandleScope scope;
  unsigned int w=args[0]->Uint32Value();
  unsigned int h=args[1]->Uint32Value();
  TwWindowSize(w,h);
  return scope.Close(Undefined());
}

JS_METHOD(AntTweakBar::Draw) {
  HandleScope scope;

  // save state
  GLint program, ab;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);
  glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &ab);
  glUseProgram(NULL);
  glBindBuffer(GL_ARRAY_BUFFER, NULL);

  // draw all AntTweakBars
  TwDraw();

  // restore state
  glUseProgram(program);
  glBindBuffer(GL_ARRAY_BUFFER, ab);

  return scope.Close(Undefined());
}

JS_METHOD(AntTweakBar::Define) {
  HandleScope scope;

  String::AsciiValue str(args[0]);
  TwDefine(*str);

  return scope.Close(Undefined());
}

JS_METHOD(AntTweakBar::NewBar) {
  HandleScope scope;

  String::AsciiValue str(args[0]);
  TwBar *bar = TwNewBar(args.Length()!=1 ? "AntTweakBar" : *str);

  return scope.Close(atb::Bar::New(bar)->handle_);
}

Persistent<FunctionTemplate> Bar::constructor_template;

void Bar::Initialize (Handle<Object> target) {
  HandleScope scope;

  Local<FunctionTemplate> t = FunctionTemplate::New(New);
  constructor_template = Persistent<FunctionTemplate>::New(t);

  constructor_template->InstanceTemplate()->SetInternalFieldCount(1);
  constructor_template->SetClassName(JS_STR("Bar"));

  NODE_SET_PROTOTYPE_METHOD(constructor_template, "AddVar", AddVar);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "RemoveVar", RemoveVar);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "RemoveAllVars", RemoveAllVars);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "AddButton", AddButton);
  NODE_SET_PROTOTYPE_METHOD(constructor_template, "AddSeparator", AddSeparator);

  target->Set(String::NewSymbol("Bar"), constructor_template->GetFunction());
}

Handle<Value> Bar::New (const Arguments& args) {
  if (!args.IsConstructCall())
    return ThrowTypeError("Constructor cannot be called as a function.");

  HandleScope scope;
  Bar *cl = new Bar(args.This());
  cl->Wrap(args.This());
  return scope.Close(args.This());
}

Bar::Bar(Handle<Object> wrapper) : bar(NULL)
{
}

Bar::~Bar () {
  if(bar) TwDeleteBar(bar);
}

Bar *Bar::New(TwBar *zbar)
{

  HandleScope scope;

  Local<Value> arg = Integer::NewFromUnsigned(0);
  Local<Object> obj = constructor_template->GetFunction()->NewInstance(1, &arg);

  Bar *v8bar = ObjectWrap::Unwrap<Bar>(obj);
  v8bar->bar = zbar;

  return v8bar;
}

struct CB {
  Persistent<Function> getter, setter;
  uint32_t type;
};

void TW_CALL SetCallback(const void *value, void *clientData) {
  //cout<<"in SetCallback"<<endl;

  HandleScope scope;
  CB *cb=static_cast<CB*>(clientData);

  Handle<Value> argv[1];

  switch(cb->type) {
  case TW_TYPE_INT8:
    argv[0]=JS_INT(*static_cast<const int8_t*>(value));
    break;
  case TW_TYPE_UINT8:
    argv[0]=JS_INT(*static_cast<const uint8_t*>(value));
    break;
  case TW_TYPE_INT16:
    argv[0]=JS_INT(*static_cast<const int16_t*>(value));
    break;
  case TW_TYPE_UINT16:
    argv[0]=JS_INT(*static_cast<const uint16_t*>(value));
    break;
  case TW_TYPE_INT32:
    argv[0]=JS_NUM(*static_cast<const int32_t*>(value));
    break;
  case TW_TYPE_UINT32:
    argv[0]=JS_NUM(*static_cast<const uint32_t*>(value));
    break;
  case TW_TYPE_FLOAT:
    argv[0]=JS_NUM(*static_cast<const float*>(value));
    break;
  case TW_TYPE_DOUBLE:
    argv[0]=JS_NUM(*static_cast<const double*>(value));
    break;
  case TW_TYPE_DIR3F: {
    const float *val=static_cast<const float*>(value);
    Local<Array> arr=Array::New(3);
    arr->Set(0,JS_NUM(val[0]));
    arr->Set(1,JS_NUM(val[1]));
    arr->Set(2,JS_NUM(val[2]));
    argv[0]=arr;
    break;
  }
  }

  TryCatch try_catch;

  Local<Value> val=cb->setter->Call(Context::GetCurrent()->Global(), 1, argv);

  if (try_catch.HasCaught())
      FatalException(try_catch);

}

void TW_CALL GetCallback(void *value, void *clientData){
  //cout<<"in GetCallback"<<endl;

  HandleScope scope;
  CB *cb=static_cast<CB*>(clientData);

  // build callback values
  Handle<Value> argv[1];
  argv[0]=Undefined();

  TryCatch try_catch;

  /*cout<<"  calling JS getter"<<endl;
  Local<Context> ctx=Context::GetCurrent();
  Local<Object> global=ctx->Global();
  cout<<"global context: "<<*ctx<<" global object: "<<*global<<endl;
  Handle<Value> name=cb->getter->GetName();
  String::AsciiValue str(name);
  cout<<"getter name: "<<*str<<" callable? "<<cb->getter->IsCallable()<<" function? "<<cb->getter->IsFunction()<<endl;
  cout<<"  global has getter()? "<<global->Has(name->ToString())<<endl;*/

  Local<Value> val=cb->getter->Call(Context::GetCurrent()->Global(), 1, argv);

  if (try_catch.HasCaught())
      FatalException(try_catch);

  switch(cb->type) {
  case TW_TYPE_INT8:
    *static_cast<int8_t *>(value) = (int8_t)val->Int32Value();
    break;
  case TW_TYPE_UINT8:
    *static_cast<uint8_t *>(value) = (uint8_t)val->Uint32Value();
    break;
  case TW_TYPE_INT16:
    *static_cast<int16_t *>(value) = (int16_t)val->Int32Value();
    break;
  case TW_TYPE_UINT16:
    *static_cast<uint16_t *>(value) = (uint16_t)val->Uint32Value();
    break;
  case TW_TYPE_INT32:
    *static_cast<int32_t *>(value) = (int32_t)val->Int32Value();
    break;
  case TW_TYPE_UINT32:
    *static_cast<uint32_t *>(value) = (uint32_t)val->Uint32Value();
    break;
  case TW_TYPE_FLOAT:
    *static_cast<float *>(value) = (float)val->NumberValue();
    break;
  case TW_TYPE_DOUBLE:
    *static_cast<double *>(value) = (double)val->NumberValue();
    break;
  case TW_TYPE_COLOR32:
    *static_cast<uint32_t *>(value) = (uint32_t)val->Uint32Value();
    break;
  case TW_TYPE_COLOR3F: {
    Local<Array> arr=Array::Cast(*val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    break;
  }
  case TW_TYPE_COLOR4F: {
    Local<Array> arr=Array::Cast(*val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    zvalue[3] = (float)arr->Get(3)->NumberValue();
    break;
  }
  case TW_TYPE_DIR3F: {
    Local<Array> arr=Array::Cast(*val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    break;
  }
  case TW_TYPE_DIR3D: {
    Local<Array> arr=Array::Cast(*val);
    double *zvalue=static_cast<double *>(value);
    zvalue[0] = (double)arr->Get(0)->NumberValue();
    zvalue[1] = (double)arr->Get(1)->NumberValue();
    zvalue[2] = (double)arr->Get(2)->NumberValue();
    break;
  }
  case TW_TYPE_QUAT4F: {
    Local<Array> arr=Array::Cast(*val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    zvalue[3] = (float)arr->Get(3)->NumberValue();
    break;
  }
  case TW_TYPE_QUAT4D: {
    Local<Array> arr=Array::Cast(*val);
    double *zvalue=static_cast<double *>(value);
    zvalue[0] = (double)arr->Get(0)->NumberValue();
    zvalue[1] = (double)arr->Get(1)->NumberValue();
    zvalue[2] = (double)arr->Get(2)->NumberValue();
    zvalue[3] = (double)arr->Get(3)->NumberValue();
    break;
  }
  }
}

JS_METHOD(Bar::AddVar) {
  HandleScope scope;
  Bar *bar = UnwrapThis<Bar>(args);
  String::AsciiValue name(args[0]);
  uint32_t type=args[1]->Uint32Value();
  Local<Array> params=Array::Cast(*args[2]);
  Local<Function> getter=Local<Function>::Cast(params->Get(JS_STR("getter")));
  Local<Function> setter=Local<Function>::Cast(params->Get(JS_STR("setter")));
  CB *callbacks=new CB();
  callbacks->type=type;
  callbacks->getter=Persistent<Function>::New(getter);
  if(!setter->IsUndefined())
    callbacks->setter=Persistent<Function>::New(setter);

  String::AsciiValue def(args[3]);
  cout<<"[AddVarRW] name="<<*name<<" type: "<<type<<" def= "<<*def<<endl;

  TwAddVarCB(bar->bar,*name,(TwType) type,setter->IsUndefined() ? NULL : atb::SetCallback, atb::GetCallback,callbacks,*def);

  // TODO Warning: must dispose persistent<functions
  // cb->getter.Dispose();
  // cb->setter.Dispose();
  // delete cb;

  return scope.Close(Undefined());
}

JS_METHOD(Bar::AddSeparator) {
  HandleScope scope;
  Bar *bar = UnwrapThis<Bar>(args);
  String::AsciiValue name(args[0]);
  String::AsciiValue def(args[1]);
  TwAddSeparator(bar->bar,args[0]->IsUndefined() ? NULL : *name,args[1]->IsUndefined() ? NULL : *def);
  return scope.Close(Undefined());
}

JS_METHOD(Bar::RemoveVar) {
  HandleScope scope;
  Bar *bar = UnwrapThis<Bar>(args);
  String::AsciiValue name(args[0]);
  TwRemoveVar(bar->bar,*name);
  return scope.Close(Undefined());
}

JS_METHOD(Bar::RemoveAllVars) {
  HandleScope scope;
  Bar *bar = UnwrapThis<Bar>(args);
  TwRemoveAllVars(bar->bar);
  return scope.Close(Undefined());
}

// TODO callback for button with user data
JS_METHOD(Bar::AddButton) {
  HandleScope scope;
  Bar *bar = UnwrapThis<Bar>(args);
  String::AsciiValue name(args[0]);
  Local<Function> cb=Local<Function>::Cast(args[1]);

  // TODO get userdata object from args[2]

  String::AsciiValue def(args[3]);

  TwAddButton(bar->bar,*name,NULL,NULL,*def);
  return scope.Close(Undefined());
}

} // namespace atb
