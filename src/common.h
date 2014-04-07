/*
 * common.h
 *
 */

#ifndef COMMON_H_
#define COMMON_H_

// OpenGL Graphics Includes
#define GLEW_STATIC
#include <GL/glew.h>

//#define GLFW_DLL
#include <GLFW/glfw3.h>

// NodeJS includes
#include <node.h>
#include "nan.h"

using namespace v8;

namespace {
#define JS_STR(...) v8::String::New(__VA_ARGS__)
#define JS_INT(val) v8::Integer::New(val)
#define JS_NUM(val) v8::Number::New(val)
#define JS_BOOL(val) v8::Boolean::New(val)
#define JS_RETHROW(tc) v8::Local<v8::Value>::New(tc.Exception());

template <typename T>
static T* UnwrapThis(const v8::Arguments& args) {
  return node::ObjectWrap::Unwrap<T>(args.This());
}

#define REQ_ARGS(N)                                                     \
  if (args.Length() < (N))                                              \
    return NanThrowTypeError("Expected " #N "arguments");

#define REQ_STR_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsString())                     \
    return NanThrowTypeError("Argument " #I " must be a string");          \
  String::Utf8Value VAR(args[I]->ToString());

#define REQ_EXT_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsExternal())                   \
    return NanThrowTypeError("Argument " #I " invalid");                   \
  Local<External> VAR = Local<External>::Cast(args[I]);

#define REQ_FUN_ARG(I, VAR)                                             \
  if (args.Length() <= (I) || !args[I]->IsFunction())                   \
    return NanThrowTypeError("Argument " #I " must be a function");  \
  Local<Function> VAR = Local<Function>::Cast(args[I]);

#define REQ_ERROR_THROW(error) if (ret == error) return NanThrowError(#error);

}
#endif /* COMMON_H_ */
