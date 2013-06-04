#include "vt.h"
#include <pwd.h>

FN(log)
    char *s = strdup(TOSTRING(args[0]));
    {
        UNLOCK;
        fprintf(stdout, "%s\n", s);
    }
    delete [] s;
    return Undefined();
ENDFN

FN(error)
    char *s = strdup(TOSTRING(args[0]));
    {
        UNLOCK;
        fprintf(stderr, "%s\n", s);
    }
    delete [] s;
    return Undefined();
ENDFN

FN(getPassword)
    char *prompt = strdup(TOSTRING(args[0])),
         *pass;
    {
        UNLOCK;
        pass = getpass(prompt);
    }
    Handle<String>s = String::New(pass);
    {
        UNLOCK;
        for (int i=0; i<_PASSWORD_LEN; i++) {
            pass[i] = '\0';
        }
    }
    delete [] prompt;
    RETURN(s);
ENDFN

Handle<ObjectTemplate>init_console() {
    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETFN(o, log);
    OSETFN(o, error);
    OSETFN(o, getPassword);
    return o;
}
