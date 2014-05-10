#include "atb.h"

#include <cstring>
#include <iostream>
using namespace std;

namespace atb {
Persistent<FunctionTemplate> AntTweakBar::constructor_template;

#define DEFINE_ATB_CONSTANT(constant) \
    NODE_DEFINE_CONSTANT_VALUE(ctor, "TYPE_" #constant, TW_TYPE_##constant);

void AntTweakBar::Initialize (Handle<Object> target) {
  NanScope();

  Local<FunctionTemplate> ctor = FunctionTemplate::New(AntTweakBar::New);
  NanAssignPersistent(FunctionTemplate, constructor_template, ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(NanSymbol("AntTweakBar"));

  NODE_SET_PROTOTYPE_METHOD(ctor, "Init", Init);
  NODE_SET_PROTOTYPE_METHOD(ctor, "Terminate", Terminate);
  NODE_SET_PROTOTYPE_METHOD(ctor, "WindowSize", WindowSize);
  NODE_SET_PROTOTYPE_METHOD(ctor, "Draw", Draw);
  NODE_SET_PROTOTYPE_METHOD(ctor, "NewBar", NewBar);
  NODE_SET_PROTOTYPE_METHOD(ctor, "Define", Define);
  NODE_SET_PROTOTYPE_METHOD(ctor, "DefineEnum", DefineEnum);

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

  target->Set(NanSymbol("AntTweakBar"), ctor->GetFunction());
}

NAN_METHOD(AntTweakBar::New) {
  if (!args.IsConstructCall())
    return NanThrowTypeError("Constructor cannot be called as a function.");

  NanScope();
  AntTweakBar *cl = new AntTweakBar(args.This());
  cl->Wrap(args.This());
  NanReturnValue(args.This());
}

AntTweakBar::~AntTweakBar () {
  TwTerminate();
}

NAN_METHOD(AntTweakBar::Init) {
  NanScope();
  TwInit(TW_OPENGL, NULL);
  NanReturnUndefined();
}

NAN_METHOD(AntTweakBar::Terminate) {
  NanScope();
  TwTerminate();
  NanReturnUndefined();
}

NAN_METHOD(AntTweakBar::WindowSize) {
  NanScope();
  unsigned int w=args[0]->Uint32Value();
  unsigned int h=args[1]->Uint32Value();
  TwWindowSize(w,h);
  NanReturnUndefined();
}

NAN_METHOD(AntTweakBar::Draw) {
  NanScope();

  // save state
  GLint program;//, ab, eab;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);
  glUseProgram(0);
  
  // draw all AntTweakBars
  TwDraw();

  // restore state
  glUseProgram(program);

  NanReturnUndefined();
}

NAN_METHOD(AntTweakBar::Define) {
  NanScope();

  String::AsciiValue str(args[0]);
  TwDefine(*str);

  NanReturnUndefined();
}

NAN_METHOD(AntTweakBar::DefineEnum) {
  NanScope();

  String::AsciiValue str(args[0]);
  Local<Array> arr=Local<Array>::Cast(args[1]);
  int num=args[2]->IsUndefined() ? arr->Length() : args[2]->Uint32Value();

  TwEnumVal *vals=new TwEnumVal[num];
  for(int i=0;i<num;i++) {
    vals[i].Value=i;
    String::AsciiValue str(arr->Get(i)->ToString());
    vals[i].Label=strdup(*str);
    //cout<<"  Adding value: "<<i<<" = "<<*str<<endl;
  }

  TwType type=TwDefineEnum(*str, vals, num);

  for(int i=0;i<num;i++)
    delete vals[i].Label;
  delete[] vals;

  NanReturnValue(JS_INT(type));
}


NAN_METHOD(AntTweakBar::NewBar) {
  NanScope();

  String::AsciiValue str(args[0]);
  TwBar *bar = TwNewBar(args.Length()!=1 ? "AntTweakBar" : *str);

  NanReturnValue(NanObjectWrapHandle(atb::Bar::New(bar)));
}

Persistent<FunctionTemplate> Bar::constructor_template;

void Bar::Initialize (Handle<Object> target) {
  NanScope();

  Local<FunctionTemplate> ctor = FunctionTemplate::New(New);
  NanAssignPersistent(FunctionTemplate, constructor_template, ctor);
  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(NanSymbol("Bar"));

  NODE_SET_PROTOTYPE_METHOD(ctor, "AddVar", AddVar);
  NODE_SET_PROTOTYPE_METHOD(ctor, "RemoveVar", RemoveVar);
  NODE_SET_PROTOTYPE_METHOD(ctor, "RemoveAllVars", RemoveAllVars);
  NODE_SET_PROTOTYPE_METHOD(ctor, "AddButton", AddButton);
  NODE_SET_PROTOTYPE_METHOD(ctor, "AddSeparator", AddSeparator);

  target->Set(NanSymbol("Bar"), ctor->GetFunction());
}

NAN_METHOD(Bar::New) {
  if (!args.IsConstructCall())
    return NanThrowTypeError("Constructor cannot be called as a function.");

  NanScope();
  Bar *cl = new Bar(args.This());
  cl->Wrap(args.This());
  NanReturnValue(args.This());
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

  NanScope();

  Local<Value> arg = Integer::NewFromUnsigned(0);
  Local<FunctionTemplate> constructorHandle = NanPersistentToLocal(constructor_template);
  Local<Object> obj = constructorHandle->GetFunction()->NewInstance(1, &arg);

  Bar *v8bar = ObjectWrap::Unwrap<Bar>(obj);
  v8bar->bar = zbar;

  return v8bar;
}

void TW_CALL SetCallback(const void *value, void *clientData) {
  //cout<<"in SetCallback"<<endl;

  NanScope();
  CB *cb=static_cast<CB*>(clientData);
  //cout<<"  cb type: "<<cb->type<<endl;

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
  default:
    // assuming custom user-defined type, this is the index
    argv[0]=JS_INT(*static_cast<const uint32_t*>(value));
    break;
  }

  TryCatch try_catch;

  Local<Function> constructorHandle = NanPersistentToLocal(cb->setter);
  constructorHandle->Call(Context::GetCurrent()->Global(), 1, argv);

  if (try_catch.HasCaught())
    FatalException(try_catch);

}

void TW_CALL GetCallback(void *value, void *clientData){
  //cout<<"in GetCallback"<<endl;

  NanScope();
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

  Local<Function> constructorHandle = NanPersistentToLocal(cb->getter);
  Local<Value> val=constructorHandle->Call(Context::GetCurrent()->Global(), 1, argv);

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

  NanScope();
  CB *cb=static_cast<CB*>(clientData);
  //cout<<"  cb type: "<<cb->type<<endl;

  Handle<Value> argv[1];
  argv[0]=Undefined();

  TryCatch try_catch;

  Local<Function> constructorHandle = NanPersistentToLocal(cb->setter);
  constructorHandle->Call(Context::GetCurrent()->Global(), 1, argv);

  if (try_catch.HasCaught())
    FatalException(try_catch);

}

NAN_METHOD(Bar::AddVar) {
  NanScope();
  Bar *bar = ObjectWrap::Unwrap<Bar>(args.This());
  String::AsciiValue name(args[0]);
  uint32_t type=args[1]->Uint32Value();
  Local<Array> params=Local<Array>::Cast(args[2]);
  Local<Function> getter=Local<Function>::Cast(params->Get(JS_STR("getter")));
  Local<Function> setter=Local<Function>::Cast(params->Get(JS_STR("setter")));
  CB *callbacks=new CB();
  bar->cbs.push_back(callbacks);
  callbacks->name=strdup(*name);
  callbacks->type=type;
  if(!getter->IsUndefined()) {
    NanInitPersistent(Function,_getter,getter);
    NanAssignPersistent(Function, callbacks->getter, _getter);
  }
  if(!setter->IsUndefined()) {
    NanInitPersistent(Function,_setter,setter);
    NanAssignPersistent(Function, callbacks->setter, _setter);
  }

  String::AsciiValue def(args[3]);
  //cout<<"[AddVarRW] name="<<*name<<" type: "<<type<<" def= "<<*def<<endl;

  TwAddVarCB(bar->bar,*name,(TwType) type,
          setter->IsUndefined() ? NULL : atb::SetCallback,
          getter->IsUndefined() ? NULL : atb::GetCallback,
          callbacks, *def);

  NanReturnUndefined();
}

NAN_METHOD(Bar::AddSeparator) {
  NanScope();
  Bar *bar = ObjectWrap::Unwrap<Bar>(args.This());
  String::AsciiValue name(args[0]);
  String::AsciiValue def(args[1]);
  TwAddSeparator(bar->bar,args[0]->IsUndefined() ? NULL : *name,args[1]->IsUndefined() ? NULL : *def);
  NanReturnUndefined();
}

NAN_METHOD(Bar::RemoveVar) {
  NanScope();
  Bar *bar = ObjectWrap::Unwrap<Bar>(args.This());
  String::AsciiValue name(args[0]);
  TwRemoveVar(bar->bar,*name);
  NanReturnUndefined();
}

NAN_METHOD(Bar::RemoveAllVars) {
  NanScope();
  Bar *bar = ObjectWrap::Unwrap<Bar>(args.This());
  TwRemoveAllVars(bar->bar);
  NanReturnUndefined();
}

NAN_METHOD(Bar::AddButton) {
  NanScope();
  Bar *bar = ObjectWrap::Unwrap<Bar>(args.This());
  String::AsciiValue name(args[0]);
  Local<Function> cb=Local<Function>::Cast(args[1]);
  String::AsciiValue def(args[2]);

  CB *callbacks=NULL;
  if(!cb->IsUndefined()) {
    callbacks=new CB();
    bar->cbs.push_back(callbacks);
    callbacks->name=strdup(*name);
    NanInitPersistent(Function,_setter,cb);
    NanAssignPersistent(Function, callbacks->setter, _setter);
  }

  TwAddButton(bar->bar,*name,
              cb->IsUndefined() ? NULL : atb::SetButtonCallback,
              callbacks,
              *def);
  NanReturnUndefined();
}

} // namespace atb
