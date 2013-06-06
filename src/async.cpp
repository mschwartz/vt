#include "vt.h"
#include <errno.h>

static inline fd_set *PTR(Handle<Value>v) {
    return (fd_set *)TOPTR(v);
}

FN(alloc_fd_set)
    fd_set *set;
    {
        UNLOCK;
        set = new fd_set;
        FD_ZERO(set);
    }
    RETURN_PTR(set);
ENDFN

FN(free_fd_set)
    fd_set *set = PTR(args[0]);
    {
        UNLOCK;
        delete set;
    }
    return Undefined();
ENDFN

FN(fd_zero)
    fd_set *set = PTR(args[0]);
    {
        UNLOCK;
        FD_ZERO(set);
    }
    return Undefined();
ENDFN

FN(fd_set)
    int fd = TOINT(args[0]);
    fd_set *set = PTR(args[1]);
    {
        UNLOCK;
        FD_SET(fd, set);
    }
    return Undefined();
ENDFN

FN(fd_clr)
    int fd = TOINT(args[0]);
    fd_set *set = PTR(args[1]);
    {
        UNLOCK;
        FD_CLR(fd, set);
    }
    return Undefined();
ENDFN

FN(fd_isset)
    int fd = TOINT(args[0]),
        ret;
    fd_set *set = PTR(args[1]);
    {
        UNLOCK;
        ret = FD_ISSET(fd, set);
    }
    return set ? True() : False();
ENDFN

FN(select)
    fd_set  readfds,
            writefds,
            exceptfds,
            *fds;

    int maxfd = TOINT(args[0]);
    if (!args[1]->IsNull() && !args[1]->IsUndefined()) {
        fds = PTR(args[1]);
        readfds = *fds;
    }
    else {
        FD_ZERO(&readfds);
    }
    if (!args[2]->IsNull() && !args[2]->IsUndefined()) {
        fds = PTR(args[2]);
        writefds = *fds;
    }
    else {
        FD_ZERO(&writefds);
    }
    if (!args[3]->IsNull() && !args[3]->IsUndefined()) {
        fds = PTR(args[3]);
        exceptfds = *fds;
    }
    else {
        FD_ZERO(&exceptfds);
    }
    int ret = select(maxfd, &readfds, &writefds, &exceptfds, NULL);
    switch (ret) {
        case 0:
            return False();
        case -1:
            return False();
    }
    Handle<Array> ra = Array::New();
    Handle<Array> wa = Array::New();
    Handle<Array> ea = Array::New();
    int randx = 0,
        wandx = 0,
        eandx = 0;

    for (int i=0; i<= maxfd; i++) {
        if (FD_ISSET(i, &readfds)) {
            ra->Set(randx++, Integer::New(i));
        }
        else if (FD_ISSET(i, &writefds)) {
            wa->Set(wandx++, Integer::New(i));
        }
        else if (FD_ISSET(i, &exceptfds)) {
            ea->Set(eandx++, Integer::New(i));
        }
    }

    Handle<Object> o = Object::New();
    OSETO(o, read, ra);
    OSETO(o, write, wa);
    OSETO(o, except, ea);
    return o;
ENDFN

FN(readable)
    int fd = TOINT(args[0]),
        sec = TOINT(args[1]),
        usec = TOINT(args[2]),
        ret;
    {
        UNLOCK;
        struct timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(
            fd+1,                           // nfds
            &fds,                           // readfds
            NULL,                           // writefds
            NULL,                           // errorfds
            (sec == -1) ? NULL : &tv        //timeout
        );
    }
    if (ret == -1) {
        THROW("readable error: %s", strerror(errno));
    }
    return ret ? True() : False();
ENDFN

FN(writable)
    int fd = TOINT(args[0]),
        sec = TOINT(args[1]),
        usec = TOINT(args[2]),
        ret;
    {
        UNLOCK;
        struct timeval tv;
        tv.tv_sec = sec;
        tv.tv_usec = usec;
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);
        ret = select(fd+1, NULL, &fds, NULL, (sec == -1) ? NULL : &tv);
    }
    if (ret == -1) {
        THROW("writable error: %s", strerror(errno));
    }
    return ret ? True() : False();
ENDFN

FN(close)
    int fd = TOINT(args[0]),
        ret;
    {
        UNLOCK;
        ret = close(fd);
    }
    RETURN_INT(ret);
ENDFN

Handle<ObjectTemplate> init_async() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETFN(o, alloc_fd_set);
    OSETFN(o, free_fd_set);
    OSETFN(o, fd_zero);
    OSETFN(o, fd_set);
    OSETFN(o, fd_clr);
    OSETFN(o, fd_isset);
    OSETFN(o, select);
    OSETFN(o, readable);
    OSETFN(o, writable);
    OSETFN(o, close);
    return o;
}
