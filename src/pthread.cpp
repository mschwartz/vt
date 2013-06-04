#include "vt.h"
#include <pthread.h>

static unsigned short nextThreadId = 0;

struct ThreadInfo {
public:
    pthread_t t;
    unsigned short threadId;
    Persistent<Object>o;
};

static pthread_key_t tls_key;

static void *runner(void *p) {
    ThreadInfo *t = (ThreadInfo *)p;
    pthread_setspecific(tls_key, t);
    pthread_detach(pthread_self());
    {
        LOCK;
        HandleScope scope(ISOLATE);
        Context::Scope context_scope(CONTEXT);
        Handle<Value>v = t->o->Get(String::New("runHandler"));
        if (v.IsEmpty()) {
            printf("No runHandler\n");
        }
        else if (!v->IsFunction()) {
            printf("No runHandler (not a function)\n");
        }
        else {
            Handle<Function>func = Handle<Function>::Cast(v);
            Handle<Value>av[1];
            av[0] = t->o;
            v = func->Call(context->Global(), 1, av);
        }
    }
    pthread_exit(NULL);
}

FN(create) 
    LOCK;
    ThreadInfo *t;
    t = new ThreadInfo;
    t->o = Persistent<Object>::New(ISOLATE, args[0]->ToObject());
    {
        UNLOCK;
        pthread_create(&t->t, NULL, runner, t);
    }
    RETURN_INT(t->threadId);
ENDFN

FN(set_tid)
    ThreadInfo *t = (ThreadInfo *)pthread_getspecific(tls_key);
    t->threadId = TOINT(args[0]);
    return Undefined();
ENDFN

FN(exit) 
    ThreadInfo *t = (ThreadInfo *)pthread_getspecific(tls_key);
    if (t) {
        LOCK;
        t->o.Dispose();
    }
    delete t;
    pthread_setspecific(tls_key, NULL);
    pthread_exit(NULL);
ENDFN

FN(tid)
    ThreadInfo *t;
    {
        UNLOCK;
        t = (ThreadInfo *)pthread_getspecific(tls_key);
    }
    if (!t) {
        return False();
    }
    RETURN_INT(t->threadId);
ENDFN

FN(t)
    char buf[128];
    sprintf(buf, "%08lx", (unsigned long)pthread_self());
    RETURN_STRING(buf);
ENDFN

Handle<ObjectTemplate>init_pthread() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETFN(o, create);
    OSETFN(o, set_tid);
    OSETFN(o, exit);
    OSETFN(o, tid);
    OSETFN(o, t);
    return o;
}
