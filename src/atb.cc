#include "atb.h"

#include <cstring>
#include <iostream>
using namespace std;

namespace atb {
Nan::Persistent<Function> AntTweakBar::constructor_template;

#define DEFINE_ATB_CONSTANT(constant) \
    NODE_DEFINE_CONSTANT_VALUE(ctor->InstanceTemplate(), "TYPE_" #constant, TW_TYPE_##constant);

NAN_MODULE_INIT(AntTweakBar::Initialize)
{
  Nan::HandleScope scope;

  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(AntTweakBar::New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("AntTweakBar"));

  Nan::SetPrototypeMethod(ctor, "Init", Init);
  Nan::SetPrototypeMethod(ctor, "Terminate", Terminate);
  Nan::SetPrototypeMethod(ctor, "WindowSize", WindowSize);
  Nan::SetPrototypeMethod(ctor, "Draw", Draw);
  Nan::SetPrototypeMethod(ctor, "NewBar", NewBar);
  Nan::SetPrototypeMethod(ctor, "Define", Define);
  Nan::SetPrototypeMethod(ctor, "DefineEnum", DefineEnum);

#define NODE_DEFINE_CONSTANT_VALUE(target, name, value)                   \
  (target)->Set(JS_STR(name),                         \
                Nan::New<v8::Integer>(value),                               \
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

  constructor_template.Reset(ctor->GetFunction());
  Nan::Set(target, JS_STR("AntTweakBar"), ctor->GetFunction());
}

NAN_METHOD(AntTweakBar::New) {
  if (!info.IsConstructCall())
    return Nan::ThrowTypeError("Constructor cannot be called as a function.");

  Nan::HandleScope scope;
  AntTweakBar *cl = new AntTweakBar(info.This());
  cl->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

AntTweakBar::AntTweakBar(Handle<Object> wrapper)
{
}

AntTweakBar::~AntTweakBar () {
  TwTerminate();
}

NAN_METHOD(AntTweakBar::Init) {
  Nan::HandleScope scope;
  TwInit(TW_OPENGL, NULL);
  return;
}

NAN_METHOD(AntTweakBar::Terminate) {
  Nan::HandleScope scope;
  TwTerminate();
  return;
}

NAN_METHOD(AntTweakBar::WindowSize) {
  Nan::HandleScope scope;
  unsigned int w=info[0]->Uint32Value();
  unsigned int h=info[1]->Uint32Value();
  TwWindowSize(w,h);
  return;
}

NAN_METHOD(AntTweakBar::Draw) {
  Nan::HandleScope scope;

  // save state
  GLint program;//, ab, eab;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);
  glUseProgram(0);

  // draw all AntTweakBars
  TwDraw();

  // restore state
  glUseProgram(program);

  return;
}

NAN_METHOD(AntTweakBar::Define) {
  Nan::HandleScope scope;

  String::Utf8Value str(info[0]);
  TwDefine(*str);

  return;
}

NAN_METHOD(AntTweakBar::DefineEnum) {
  Nan::HandleScope scope;

  String::Utf8Value str(info[0]);
  Local<Array> arr=Local<Array>::Cast(info[1]);
  int num=info[2]->IsUndefined() ? arr->Length() : info[2]->Uint32Value();

  TwEnumVal *vals=new TwEnumVal[num];
  for(int i=0;i<num;i++) {
    vals[i].Value=i;
    String::Utf8Value str(arr->Get(i)->ToString());
    vals[i].Label=strdup(*str);
    //cout<<"  Adding value: "<<i<<" = "<<*str<<endl;
  }

  TwType type=TwDefineEnum(*str, vals, num);

  for(int i=0;i<num;i++)
    delete vals[i].Label;
  delete[] vals;

  info.GetReturnValue().Set(JS_INT(type));
}


NAN_METHOD(AntTweakBar::NewBar) {
  Nan::HandleScope scope;

  String::Utf8Value str(info[0]);
  TwBar *bar = TwNewBar(info.Length()!=1 ? "AntTweakBar" : *str);

  info.GetReturnValue().Set(atb::Bar::New(bar)->handle());
}

Nan::Persistent<Function> Bar::constructor_template;

NAN_MODULE_INIT(Bar::Initialize)
{
  Nan::HandleScope scope;

  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(Bar::New);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(JS_STR("Bar"));

  Nan::SetPrototypeMethod(ctor, "AddVar", AddVar);
  Nan::SetPrototypeMethod(ctor, "RemoveVar", RemoveVar);
  Nan::SetPrototypeMethod(ctor, "RemoveAllVars", RemoveAllVars);
  Nan::SetPrototypeMethod(ctor, "AddButton", AddButton);
  Nan::SetPrototypeMethod(ctor, "AddSeparator", AddSeparator);

  constructor_template.Reset(ctor->GetFunction());
  Nan::Set(target, JS_STR("Bar"), ctor->GetFunction());
}

NAN_METHOD(Bar::New) {
  if (!info.IsConstructCall())
    return Nan::ThrowTypeError("Constructor cannot be called as a function.");

  Nan::HandleScope scope;
  Bar *cl = new Bar(info.This());
  cl->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

Bar::Bar(Handle<Object> wrapper) : bar(NULL)
{
}

Bar::~Bar () {
  for(vector<CB*>::iterator it=cbs.begin();it!=cbs.end();++it) {
    CB *cb=*it;
    if(cb) delete cb;
  }
  cbs.clear();
  if(bar) TwDeleteBar(bar);
}

Bar *Bar::New(TwBar *zbar)
{

  Nan::HandleScope scope;

  Local<Function> cons = Nan::New<Function>(constructor_template);
  Local<Object> obj = cons->NewInstance();

  Bar *v8bar = Nan::ObjectWrap::Unwrap<Bar>(obj);
  v8bar->bar = zbar;

  return v8bar;
}

void TW_CALL SetCallback(const void *value, void *clientData) {
  // cout<<"in SetCallback"<<endl;

  Nan::HandleScope scope;
  CB *cb=static_cast<CB*>(clientData);
  // cout<<"  cb type: "<<cb->type<<endl;

  Local<Value> argv[1];

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
    Local<Array> arr=Nan::New<Array>(3);
    arr->Set(0,JS_NUM(val[0]));
    arr->Set(1,JS_NUM(val[1]));
    arr->Set(2,JS_NUM(val[2]));
    argv[0]=arr;
    break;
  }
  default:
    // assuming custom user-defined type, this is the index
    argv[0]=JS_INT(*static_cast<const uint32_t*>(value));
    break;
  }

  TryCatch try_catch;

  Local<Function> constructorHandle = Nan::New(cb->setter);
  constructorHandle->Call(v8::Isolate::GetCurrent()->GetCurrentContext()->Global(), 1, argv);

  if (try_catch.HasCaught())
    FatalException(v8::Isolate::GetCurrent(),try_catch);

}

void TW_CALL GetCallback(void *value, void *clientData) {
  // cout<<"in GetCallback"<<endl;

  Nan::HandleScope scope;
  CB *cb=static_cast<CB*>(clientData);

  // build callback values
  Local<Value> argv[1];
  argv[0]=Nan::Undefined();

  TryCatch try_catch;

  // cout<<"  calling JS getter"<<endl;
  // Local<Context> ctx=Context::GetCurrent();
  // Local<Object> global=ctx->Global();
  // cout<<"global context: "<<*ctx<<" global object: "<<*global<<endl;
  Local<Function> fct=Nan::New(cb->getter);
  // Handle<Value> name=fct->GetName();
  // String::Utf8Value str(name);
  // cout<<"getter name: "<<*str<<" callable? "<<fct->IsCallable()<<" function? "<<fct->IsFunction()<<endl;
  // cout<<"  global has getter()? "<<global->Has(name->ToString())<<endl;

  Local<Value> val=fct->Call(v8::Isolate::GetCurrent()->GetCurrentContext()->Global(), 1, argv);

  if (try_catch.HasCaught())
      FatalException(v8::Isolate::GetCurrent(),try_catch);

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
    Local<Array> arr=Local<Array>::Cast(val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    break;
  }
  case TW_TYPE_COLOR4F: {
    Local<Array> arr=Local<Array>::Cast(val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    zvalue[3] = (float)arr->Get(3)->NumberValue();
    break;
  }
  case TW_TYPE_DIR3F: {
    Local<Array> arr=Local<Array>::Cast(val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    break;
  }
  case TW_TYPE_DIR3D: {
    Local<Array> arr=Local<Array>::Cast(val);
    double *zvalue=static_cast<double *>(value);
    zvalue[0] = (double)arr->Get(0)->NumberValue();
    zvalue[1] = (double)arr->Get(1)->NumberValue();
    zvalue[2] = (double)arr->Get(2)->NumberValue();
    break;
  }
  case TW_TYPE_QUAT4F: {
    Local<Array> arr=Local<Array>::Cast(val);
    float *zvalue=static_cast<float *>(value);
    zvalue[0] = (float)arr->Get(0)->NumberValue();
    zvalue[1] = (float)arr->Get(1)->NumberValue();
    zvalue[2] = (float)arr->Get(2)->NumberValue();
    zvalue[3] = (float)arr->Get(3)->NumberValue();
    break;
  }
  case TW_TYPE_QUAT4D: {
    Local<Array> arr=Local<Array>::Cast(val);
    double *zvalue=static_cast<double *>(value);
    zvalue[0] = (double)arr->Get(0)->NumberValue();
    zvalue[1] = (double)arr->Get(1)->NumberValue();
    zvalue[2] = (double)arr->Get(2)->NumberValue();
    zvalue[3] = (double)arr->Get(3)->NumberValue();
    break;
  }
  default:
    // assuming custom user-defined type, this is the index
    *static_cast<uint32_t *>(value) = (uint32_t)val->Uint32Value();
    break;
  }
}

void TW_CALL SetButtonCallback(void *clientData) {
  //cout<<"in SetButtonCallback"<<endl;

  Nan::HandleScope scope;
  CB *cb=static_cast<CB*>(clientData);
  //cout<<"  cb type: "<<cb->type<<endl;

  Local<Value> argv[1];
  argv[0]=Nan::Undefined();

  TryCatch try_catch;

  Local<Function> constructorHandle = Nan::New(cb->setter);
  constructorHandle->Call(v8::Isolate::GetCurrent()->GetCurrentContext()->Global(), 1, argv);

  if (try_catch.HasCaught())
    FatalException(v8::Isolate::GetCurrent(),try_catch);

}

NAN_METHOD(Bar::AddVar) {
  Nan::HandleScope scope;
  Bar *bar = Nan::ObjectWrap::Unwrap<Bar>(info.This());
  String::Utf8Value name(info[0]);
  uint32_t type=info[1]->Uint32Value();
  Local<Array> params=Local<Array>::Cast(info[2]);
  Local<Function> getter=Local<Function>::Cast(params->Get(JS_STR("getter")));
  Local<Function> setter=Local<Function>::Cast(params->Get(JS_STR("setter")));
  CB *callbacks=new CB();
  bar->cbs.push_back(callbacks);
  callbacks->name=strdup(*name);
  callbacks->type=type;
  if(!getter->IsUndefined()) {
    callbacks->getter.Reset( getter);
    // cout<<"[AddVarRW] adding getter "<<endl;
  }
  if(!setter->IsUndefined()) {
    callbacks->setter.Reset( setter);
    // cout<<"[AddVarRW] adding setter "<<endl;
  }

  String::Utf8Value def(info[3]);
  // cout<<"[AddVarRW] name="<<*name<<" type: "<<type<<" def= "<<*def<<endl;

  TwAddVarCB(bar->bar,*name,(TwType) type,
          setter->IsUndefined() ? NULL : atb::SetCallback,
          getter->IsUndefined() ? NULL : atb::GetCallback,
          callbacks, *def);

  return;
}

NAN_METHOD(Bar::AddSeparator) {
  Nan::HandleScope scope;
  Bar *bar = Nan::ObjectWrap::Unwrap<Bar>(info.This());
  String::Utf8Value name(info[0]);
  String::Utf8Value def(info[1]);
  TwAddSeparator(bar->bar,info[0]->IsUndefined() ? NULL : *name,info[1]->IsUndefined() ? NULL : *def);
  return;
}

NAN_METHOD(Bar::RemoveVar) {
  Nan::HandleScope scope;
  Bar *bar = Nan::ObjectWrap::Unwrap<Bar>(info.This());
  String::Utf8Value name(info[0]);
  TwRemoveVar(bar->bar,*name);
  return;
}

NAN_METHOD(Bar::RemoveAllVars) {
  Nan::HandleScope scope;
  Bar *bar = Nan::ObjectWrap::Unwrap<Bar>(info.This());
  TwRemoveAllVars(bar->bar);
  return;
}

NAN_METHOD(Bar::AddButton) {
  Nan::HandleScope scope;
  Bar *bar = Nan::ObjectWrap::Unwrap<Bar>(info.This());
  String::Utf8Value name(info[0]);
  Local<Function> cb=Local<Function>::Cast(info[1]);
  String::Utf8Value def(info[2]);

  CB *callbacks=NULL;
  if(!cb->IsUndefined()) {
    callbacks=new CB();
    bar->cbs.push_back(callbacks);
    callbacks->name=strdup(*name);
    callbacks->setter.Reset( cb);
  }

  TwAddButton(bar->bar,*name,
              cb->IsUndefined() ? NULL : atb::SetButtonCallback,
              callbacks,
              *def);
  return;
}

} // namespace atb
