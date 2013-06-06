#include "vt.h"
#include <errno.h>

FN(error)
    char *s;
    {
        UNLOCK;
        s = strerror(errno);
    }
    RETURN_STRING(s);
ENDFN

FN(getpid)
    int ret;
    {
        UNLOCK;
        ret = getpid();
    }
    RETURN_INT(ret);
ENDFN

FN(fork)
    int pid;
    {
        UNLOCK;
        pid = fork();
    }
    RETURN_INT(pid);
ENDFN

FN(exit)
    int code = 0;
    if (args.Length()) {
        code = TOINT(args[0]);
    }
    exit(code);
ENDFN

FN(sleep)
    int secs = TOINT(args[0]);
    {
        UNLOCK;
        sleep(secs);
    }
    return Undefined();
ENDFN

FN(usleep)
    int usecs = TOINT(args[0]);
    {
        UNLOCK;
        usleep(usecs);
    }
    return Undefined();
ENDFN

Handle<ObjectTemplate>init_process() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    
    OSETFN(o, error);
    OSETFN(o, getpid);
    OSETFN(o, fork);
    OSETFN(o, exit);
    OSETFN(o, sleep);
    OSETFN(o, usleep);

    return o;
}
