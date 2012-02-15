/*
 * common.h
 *
 *  Created on: Dec 13, 2011
 *      Author: ngk437
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <node.h>

using namespace v8;

namespace {
#define JS_STR(...) v8::String::New(__VA_ARGS__)
#define JS_INT(val) v8::Integer::New(val)
#define JS_NUM(val) v8::Number::New(val)
#define JS_BOOL(val) v8::Boolean::New(val)
#define JS_METHOD(name) v8::Handle<v8::Value> name(const v8::Arguments& args)
#define JS_RETHROW(tc) v8::Local<v8::Value>::New(tc.Exception());

template <typename T>
static T* UnwrapThis(const v8::Arguments& args) {
  return node::ObjectWrap::Unwrap<T>(args.This());
}

template <typename T>
static T* UnwrapField(const Arguments &args, const int fid) {
  return (T*) External::Unwrap(args.Holder()->GetInternalField(fid));
}

template <typename T>
static T* UnwrapField(const AccessorInfo &args, const int fid) {
  return (T*) External::Unwrap(args.Holder()->GetInternalField(fid));
}

v8::Handle<v8::Value> ThrowError(const char* msg) {
  return v8::ThrowException(v8::Exception::Error(v8::String::New(msg)));
}

v8::Handle<v8::Value> ThrowTypeError(const char* msg) {
  return v8::ThrowException(v8::Exception::TypeError(v8::String::New(msg)));
}

v8::Handle<v8::Value> ThrowRangeError(const char* msg) {
  return v8::ThrowException(v8::Exception::RangeError(v8::String::New(msg)));
}

#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return ThrowTypeError("Expected " #N "arguments");

#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return ThrowTypeError("Argument " #I " must be a string");          \
  String::Utf8Value VAR(args[I]->ToString());

#define REQ_EXT_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsExternal())                   \
    return ThrowTypeError("Argument " #I " invalid");                   \
  Local<External> VAR = Local<External>::Cast(args[I]);

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return ThrowTypeError("Argument " #I " must be a function");  \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_ERROR_THROW(error) if (ret == error) return ThrowError(#error);

}
#endif /* COMMON_H_ */
