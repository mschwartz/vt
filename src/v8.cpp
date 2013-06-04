#include "vt.h"
#include <v8-debug.h>

struct ScriptWrapper {
    Persistent<Script>script;
};

FN(gc)
    for (int i=0; i<10000; i++) {
        if (V8::IdleNotification()) {
            break;
        }
    }
    return Undefined();
ENDFN

FN(compileScript)
    ScriptWrapper *wrapper = new ScriptWrapper;
    wrapper->script = Persistent<Script>::New(ISOLATE, Script::New(args[0]->ToString(), args[1]->ToString()));
    RETURN_PTR(wrapper);
ENDFN

FN(runScript)
    ScriptWrapper *wrapper = (ScriptWrapper *)TOPTR(args[0]);
    Handle<Value>v = wrapper->script->Run();
    RETURN(v);
ENDFN

FN(freeScript) 
    ScriptWrapper *wrapper = (ScriptWrapper *)TOPTR(args[0]);
    wrapper->script.Dispose();
    delete wrapper;
    return Undefined();
ENDFN

static void debugger () {
    Context::Scope scope(CONTEXT);
    Debug::ProcessDebugMessages();
}

FN(enableDebugger)
    Debug::SetDebugMessageDispatchHandler(debugger, true);
    Debug::EnableAgent("vt", 5858, true);
    Debug::DebugBreak();
    return Undefined();
ENDFN

Handle<ObjectTemplate>init_v8() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETFN(o, gc);
    OSETFN(o, compileScript);
    OSETFN(o, runScript);
    OSETFN(o, freeScript);
    OSETFN(o, enableDebugger);
    return o;
}
