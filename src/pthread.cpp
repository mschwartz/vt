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

//                  _            
//                 | |           
//  _ __ ___  _   _| |_ _____  __
// | '_ ` _ \| | | | __/ _ \ \/ /
// | | | | | | |_| | ||  __/>  < 
// |_| |_| |_|\__,_|\__\___/_/\_\
//

FN(mutex_init)
    pthread_mutex_t *mutex;
    {
        UNLOCK;
        mutex = new pthread_mutex_t;
        pthread_mutex_init(mutex, NULL);
    }
    RETURN_PTR(mutex);
ENDFN

FN(mutex_destroy)
    pthread_mutex_t *mutex = (pthread_mutex_t *)TOPTR(args[0]);
    {
        UNLOCK;
        pthread_mutex_destroy(mutex);
        delete mutex;
    }
    return Undefined();
ENDFN

FN(mutex_lock)
    pthread_mutex_t *mutex = (pthread_mutex_t *)TOPTR(args[0]);
    int ret;
    {
        UNLOCK;
        ret = pthread_mutex_lock(mutex);
    }
    RETURN_INT(ret);
ENDFN

FN(mutex_trylock)
    pthread_mutex_t *mutex = (pthread_mutex_t *)TOPTR(args[0]);
    int ret;
    {
        UNLOCK;
        ret = pthread_mutex_trylock(mutex);
    }
    RETURN_INT(ret);
ENDFN

FN(mutex_unlock)
    pthread_mutex_t *mutex = (pthread_mutex_t *)TOPTR(args[0]);
    int ret;
    {
        UNLOCK;
        ret = pthread_mutex_unlock(mutex);
    }
    RETURN_INT(ret);
ENDFN

//                      _ _ _   _
//                     | (_) | (_)
//   ___ ___  _ __   __| |_| |_ _  ___  _ __  ___ 
//  / __/ _ \| '_ \ / _` | | __| |/ _ \| '_ \/ __|
// | (_| (_) | | | | (_| | | |_| | (_) | | | \__ \
//  \___\___/|_| |_|\__,_|_|\__|_|\___/|_| |_|___/
//                                             

FN(cond_init)
    pthread_cond_t *cond;
    {
        UNLOCK;
        cond = new pthread_cond_t;
        pthread_cond_init(cond, NULL);
    }
    RETURN_PTR(cond);
ENDFN

FN(cond_destroy)
    pthread_cond_t *cond = (pthread_cond_t *)TOPTR(args[0]);
    {
        UNLOCK;
        pthread_cond_destroy(cond);
        delete cond;
    }
ENDFN

FN(cond_wait)
    pthread_cond_t *cond = (pthread_cond_t *)TOPTR(args[0]);
    pthread_mutex_t *mutex = (pthread_mutex_t *)TOPTR(args[1]);
    int ret;
    {
        UNLOCK;
        ret = pthread_cond_wait(cond, mutex);
    }
    RETURN_INT(ret);
ENDFN

FN(cond_signal)
    pthread_cond_t *cond = (pthread_cond_t *)TOPTR(args[0]);
    int ret;
    {
        UNLOCK;
        ret = pthread_cond_signal(cond);
    }
    RETURN_INT(ret);
ENDFN

FN(cond_broadcast)
    pthread_cond_t *cond = (pthread_cond_t *)TOPTR(args[0]);
    int ret;
    {
        UNLOCK;
        ret = pthread_cond_broadcast(cond);
    }
    RETURN_INT(ret);
ENDFN


Handle<ObjectTemplate>init_pthread() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    
    OSETFN(o, create);
    OSETFN(o, set_tid);
    OSETFN(o, exit);
    OSETFN(o, tid);
    OSETFN(o, t);

    OSETFN(o, mutex_init);
    OSETFN(o, mutex_destroy);
    OSETFN(o, mutex_lock);
    OSETFN(o, mutex_trylock);
    OSETFN(o, mutex_unlock);

    OSETFN(o, cond_init);
    OSETFN(o, cond_destroy);
    OSETFN(o, cond_wait);
    OSETFN(o, cond_signal);
    OSETFN(o, cond_broadcast);

    return o;
}
