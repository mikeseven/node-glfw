/*
 * atb.h
 *
 */

#ifndef ATB_H_
#define ATB_H_

#include "common.h"

#include <AntTweakBar.h>
#include <vector>
#include <cstdlib>

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
  static NAN_METHOD(New);
  static NAN_METHOD(AddVar);
  static NAN_METHOD(AddSeparator);
  static NAN_METHOD(AddButton);
  static NAN_METHOD(RemoveVar);
  static NAN_METHOD(RemoveAllVars);

  virtual ~Bar ();

private:
  Bar(Handle<Object> wrapper);
  static Persistent<Function> constructor_template;

  TwBar *bar;
  std::vector<CB*> cbs;
};

class AntTweakBar : public ObjectWrap {
public:
  static void Initialize (Handle<Object> target);

protected:
  static NAN_METHOD(New);
  static NAN_METHOD(Init);
  static NAN_METHOD(Terminate);
  static NAN_METHOD(WindowSize);
  static NAN_METHOD(Draw);
  static NAN_METHOD(Define);
  static NAN_METHOD(DefineEnum);

  static NAN_METHOD(NewBar);

  virtual ~AntTweakBar ();

private:
  AntTweakBar(Handle<Object> wrapper);
  static Persistent<Function> constructor_template;
};

} // namespace atb

#endif /* ATB_H_ */
