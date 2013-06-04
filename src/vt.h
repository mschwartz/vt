#ifndef VT_H
#define VT_H

// #include <sys/types.h>
// #include <string.h>
// #include <strings.h>
// #include <sys/file.h>
// #include <errno.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <stdio.h>
// #include <fcntl.h>
// #include <dirent.h>
// #include <sys/stat.h>
// #include <pwd.h>
// #include <string>

#include <v8.h>
#include <platform.h>

using namespace v8;

extern Handle<Context> context;
extern Isolate *main_isolate;

#define ISOLATE main_isolate
#define CONTEXT context

#define LOCK Locker l(main_isolate)
#define UNLOCK Unlocker ul(main_isolate)

class _PTR {
public:
    static Handle<Object>New(void *p) {
        Handle<ObjectTemplate>t = ObjectTemplate::New();
        t->SetInternalFieldCount(1);
        Local<Object>o = t->NewInstance();
        o->SetInternalField(0, External::New(p));
        return o;
    }
};


#define FN(a) static Handle<Value>_FN_ ## a(const Arguments& args) { HandleScope hscope(ISOLATE);
#define ENDFN }
#define RETURN(v) return hscope.Close(v)
#define RETURN_INT(n) return hscope.Close(Integer::New(n))
#define RETURN_STRING(n) return hscope.Close(String::New(n))
#define RETURN_PTR(v) return hscope.Close(_PTR::New(v))

static inline Handle<Value> THROW(const char *fmt, ...) {
    char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    {
        UNLOCK;
        vsprintf(buf, fmt, ap);
    }
    Handle<String>s = String::New(buf);
    return ThrowException(s);
}
// conversion of arg to various types
#define TOINT(arg) arg->IntegerValue()
static inline void *TOPTR(Handle<Value>v) {
    Local<External>wrap = Local<External>::Cast(v->ToObject()->GetInternalField(0));
    return wrap->Value();
}
static char *TOSTRING(Handle<Value>arg) {
    String::AsciiValue s(arg);
    return *s;
}

#define OSETFN(o, fn) o->Set(String::NewSymbol(#fn), FunctionTemplate::New(_FN_ ## fn))
#define OSETO(o, k, v) o->Set(String::NewSymbol(#k), v)
#define OSETINT(o, k, v) o->Set(String::NewSymbol(#k), Integer::New(v))
#define OSETCONST(o, c) o->Set(String::NewSymbol(#c), Integer::New(c))
#define OSETSTRING(o, k, v) o->Set(String::NewSymbol(#k), String::New(v))

#endif
