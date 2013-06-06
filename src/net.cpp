#include "vt.h"
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netdb.h>

FN(error)
    RETURN_STRING(strerror(errno));
ENDFN

FN(socket)
    int domain = TOINT(args[0]),
        type = TOINT(args[1]),
        protocol = TOINT(args[2]),
        sock;
    {
        UNLOCK;
        sock = socket(domain, type, protocol);
    }
    RETURN_INT(sock);
ENDFN

FN(listen)
    int fd = TOINT(args[0]),
        port = TOINT(args[1]),
        listenAddress = TOINT(args[2]),
        backlog = TOINT(args[3]);

    {
        UNLOCK;
        int on = 1;
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on));
        struct sockaddr_in my_addr;
        bzero(&my_addr, sizeof (my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(port);
        my_addr.sin_addr.s_addr = listenAddress; // htonl(listenAddress);
        if (bind(fd, (struct sockaddr *) &my_addr, sizeof (my_addr))) {
            LOCK;
            THROW("bind() Error: %s", strerror(errno));
        }
        if (listen(fd, backlog)) {
            LOCK;
            THROW("listen() Error: %s", strerror(errno));
        }
    }
    return Undefined();
ENDFN

FN(accept)
    int sock = TOINT(args[0]),
        fd;
    struct sockaddr_in their_addr;

    {
        UNLOCK;
        socklen_t sock_size = sizeof(struct sockaddr_in);
        fd = accept(sock, (struct sockaddr *)&their_addr, &sock_size);

#ifdef __APPLE__
        int flag = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));
#endif
    }
    Handle<Object>o = Object::New();
    OSETINT(o, fd, fd);
    OSETSTRING(o, remote_addr, inet_ntoa(their_addr.sin_addr));
    RETURN(o);
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

FN(write)
    int fd = TOINT(args[0]),
        len = TOINT(args[2]),
        ret;
    char *s = strdup(TOSTRING(args[1]));
    {
        UNLOCK;
        ret = write(fd, s, len);
        delete [] s;
    }
    RETURN_INT(ret);
ENDFN


#ifdef __APPLE__
#define TCP_CORK TCP_NODELAY
// #define TCP_CORK TCP_NOPUSH
FN(cork)
    return Undefined();
ENDFN
#else
FN(cork) 
    int fd = TOINT(args[0]),
        flag = TOINT(args[1]),
        ret;
    {
        UNLOCK;
        ret = setsockopt(fd, IPPROTO_TCP, TCP_CORK, &flag, sizeof(flag));
    }
    RETURN_INT(ret);
ENDFN
#endif

Handle<ObjectTemplate>init_net() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    
    OSETCONST(o, AF_INET);
    OSETCONST(o, SOCK_STREAM);
    OSETCONST(o, INADDR_ANY);

    OSETFN(o, error);
    OSETFN(o, socket);
    OSETFN(o, listen);
    OSETFN(o, accept);
    OSETFN(o, close);
    OSETFN(o, write);
    OSETFN(o, cork);
    
    return o;
}
