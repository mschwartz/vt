#include "vt.h"
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string>

FN(error) 
    RETURN_STRING(strerror(errno));
ENDFN

FN(chdir)
    RETURN_INT(chdir(TOSTRING(args[0])));
ENDFN

FN(getcwd)
    char *cwd = getcwd(NULL, 0);
    Handle<String>s = String::New(cwd);
    delete [] cwd;
    RETURN(s);
ENDFN

FN(realpath)
    char *path = strdup(TOSTRING(args[0])),
         *absolutePath;
    {
        UNLOCK;
        absolutePath = realpath(path, NULL);
    }
    delete [] path;
    if (!absolutePath) {
        return False();
    }
    Handle<String>s = String::New(absolutePath);
    delete [] absolutePath;
    RETURN(s);
ENDFN

FN(isFile)
    char *path = strdup(TOSTRING(args[0]));
    struct stat buf;
    bool ret = false;
    {
        UNLOCK;
        if (!stat(path, &buf) && S_ISREG(buf.st_mode)) {
            ret = true;
        }
    }
    delete [] path;
    return ret ? True() : False();
ENDFN

FN(isDir)
    char *path = strdup(TOSTRING(args[0]));
    struct stat buf;
    bool ret = false;
    {
        UNLOCK;
        if (!stat(path, &buf) && S_ISDIR(buf.st_mode)) {
            ret = true;
        }
    }
    delete [] path;
    return ret ? True() : False();
ENDFN

FN(readFile)
    char *path = strdup(TOSTRING(args[0]));
    bool ret = true;
    std::string s;
    {
        UNLOCK;
        int fd = open(path, O_RDONLY);
        delete [] path;
        if (fd != -1) {
            flock(fd, LOCK_SH);
            lseek(fd, 0, 0);
            char buf[1024];
            ssize_t count;
            while ((count = read(fd, buf, 1024))) {
                s = s.append(buf, count);
            }
            flock(fd, LOCK_UN);
            close(fd);
        }
        else {
            ret = false;
        }
    }
    if (ret) {
        return hscope.Close(String::New(s.c_str(), s.size()));
        // RETURN(String::New(s.c_str(), s.size()));
    }
    else {
printf("NULL\n");
        return Null();
    }
ENDFN

Handle<ObjectTemplate>init_fs() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETFN(o, error);
    OSETFN(o, chdir);
    OSETFN(o, getcwd);
    OSETFN(o, realpath);
    OSETFN(o, isFile);
    OSETFN(o, isDir);
    OSETFN(o, readFile);
    return o;
}
