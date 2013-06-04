#include "vt.h"

struct MEMINFO {
    int size;
    unsigned char *mem;
};


FN(alloc) 
    int size = TOINT(args[0]);
    MEMINFO *m;
    {
        UNLOCK;
        m = new MEMINFO;
        if (!m) {
            return Null();
        }
        m->size = size;
        m->mem = new unsigned char[size];
    }
    if (!m->mem) {
        delete m;
        return Null();
    }
    RETURN_PTR(m);
ENDFN

FN(free)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    {
        UNLOCK;
        delete [] m->mem;
        delete m;
    }
    return Undefined();
ENDFN

FN(size)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    RETURN_INT(m->size);
ENDFN

FN(realloc)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    int new_size = TOINT(args[1]);
    unsigned char *n;
    {
        UNLOCK;
        n = (unsigned char *)realloc(m->mem, new_size);
    }
    if (!n) {
        return Null();
    }
    m->mem = n;
    m->size = new_size;
    RETURN_PTR(m);
ENDFN

FN(dup)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]),
            *n;
    {
        UNLOCK;
        n = new MEMINFO;
        n->size = m->size;
        n->mem = new unsigned char[n->size];
        memcpy(n->mem, m->mem, n->size);
    }
    RETURN_PTR(n);
ENDFN

FN(substr)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]),
            *n;
    int offset = TOINT(args[1]);
    int size = m->size - offset;
    if (args.Length() > 2) {
        size = TOINT(args[2]);
    }
    {
        UNLOCK;
        n = new MEMINFO;
        if (!n) {
            return Null();
        }
        n->size = size;
        n->mem = new unsigned char[size];
        if (!n->mem) {
            delete n;
            return Null();
        }
        memcpy(n->mem, &m->mem[offset], size);
    }
    RETURN_PTR(n);
ENDFN

FN(asString) 
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    int offset = TOINT(args[1]),
        size = TOINT(args[2]);
    RETURN(String::New((const char *)&m->mem[offset], size));
ENDFN

FN(read)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    int fd = TOINT(args[1]),
        offset = TOINT(args[2]),
        size = TOINT(args[3]),
        n;
    {
        UNLOCK;
        n = read(fd, &m->mem[offset], size);
    }
    RETURN_INT(n);
ENDFN

FN(write)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    int fd = TOINT(args[1]),
        offset = TOINT(args[2]),
        size = TOINT(args[3]),
        n;
    {
        UNLOCK;
        n = write(fd, &m->mem[offset], size);
    }
    RETURN_INT(n);
ENDFN

FN(getat)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    int offset = TOINT(args[1]);
    RETURN(String::New((const char *)&m->mem[offset], 1));
ENDFN

FN(append)
    MEMINFO *m = (MEMINFO *)TOPTR(args[0]);
    int offset = TOINT(args[2]),
        length = TOINT(args[3]);
    memcpy(&m->mem[offset], TOSTRING(args[1]), length);
    return Undefined();
ENDFN

Handle<ObjectTemplate>init_mem() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETFN(o, alloc);
    OSETFN(o, free);
    OSETFN(o, size);
    OSETFN(o, realloc);
    OSETFN(o, dup);
    OSETFN(o, substr);
    OSETFN(o, asString);
    OSETFN(o, read);
    OSETFN(o, write);
    OSETFN(o, getat);
    OSETFN(o, append);
    return o;
}
