/*
 * atb.h
 *
 *  Created on: Mar 8, 2012
 *      Author: ngk437
 */

#ifndef ATB_H_
#define ATB_H_

#include "common.h"

#include <AntTweakBar.h>
#include <vector>

using namespace v8;
using namespace node;

namespace atb {

struct CB {
  Persistent<Function> getter, setter;
  uint32_t type;
  char *name;
  CB() : type(0), name(NULL) {}
  ~CB() {
    getter.Dispose();
    setter.Dispose();
    if(name) free(name);
  }
};

class Bar : public ObjectWrap {
public:
  static Bar *New(TwBar *bar);
  static void Initialize (Handle<Object> target);

protected:
  static Handle<Value> New (const Arguments& args);

  static JS_METHOD(AddVar);
  static JS_METHOD(AddSeparator);
  static JS_METHOD(AddButton);
  static JS_METHOD(RemoveVar);
  static JS_METHOD(RemoveAllVars);

  virtual ~Bar ();

private:
  Bar(Handle<Object> wrapper);
  static Persistent<FunctionTemplate> constructor_template;

  TwBar *bar;
  std::vector<CB*> cbs;
};

class AntTweakBar : public ObjectWrap {
public:
  static void Initialize (Handle<Object> target);

protected:
  static Handle<Value> New (const Arguments& args);
  static JS_METHOD(Init);
  static JS_METHOD(Terminate);
  static JS_METHOD(WindowSize);
  static JS_METHOD(Draw);
  static JS_METHOD(Define);

  static JS_METHOD(NewBar);

  virtual ~AntTweakBar ();

private:
  AntTweakBar(Handle<Object> wrapper);
  static Persistent<FunctionTemplate> constructor_template;
};

} // namespace atb

#endif /* ATB_H_ */
