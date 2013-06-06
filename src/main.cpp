#include "vt.h"
#include <stdio.h>
#include <fcntl.h>
#include <sys/time.h>

Handle<Context> context;
Isolate *main_isolate;

char *readFile (const char *fn) {
    int fd = open(fn, O_RDONLY);
    if (fd < 0) {
        return NULL;
    }
    long size = lseek(fd, 0, 2);
    lseek(fd, 0, 0);
    char *file = new char[size + 1];
    size = read(fd, file, size);
    file[size] = '\0';
    close(fd);
    return file;
}

FN(timer)
    timeval t;
    gettimeofday(&t, NULL);
    RETURN_INT(t.tv_sec * 1000000 + t.tv_usec);
    // RETURN_INT(mach_absolute_time() & 0x7fffffff);
ENDFN

FN(log)
    char *s = strdup(TOSTRING(args[0]));
	{
        UNLOCK;
        printf("%08lx %s\n", (unsigned long)pthread_self(), s);
        delete [] s;
        sleep(0);
    }
    return Undefined();
ENDFN

FN(include)
    for (int i = 0; i < args.Length(); i++) {
        String::Utf8Value str(args[i]);
        char buf[strlen(*str) + 18 + 1];
        strcpy(buf, *str);
        char *js_file = readFile(*str);
        if (!js_file) {
            strcpy(buf, *str);
            if (buf[0] != '/') {
                strcpy(buf, "/usr/local/silkjs/");
                strcat(buf, *str);
            }
            js_file = readFile(buf);
        }
        if (!js_file) {
            return ThrowException(String::Concat(String::New("include file not found "), args[i]->ToString()));
        }
        Handle<String> source = String::New(js_file);
        delete [] js_file;
        ScriptOrigin origin(String::New(*str), Integer::New(0), Integer::New(0));
        Handle<Script>script = Script::New(source, &origin);
        script->Run();
    }
    return Undefined();
ENDFN

Handle<ObjectTemplate>builtins() {
    extern Handle<ObjectTemplate>init_console();
    extern Handle<ObjectTemplate>init_process();
    extern Handle<ObjectTemplate>init_v8();
    extern Handle<ObjectTemplate>init_mem();
    extern Handle<ObjectTemplate>init_pthread();
    extern Handle<ObjectTemplate>init_fs();
    extern Handle<ObjectTemplate>init_net();
    extern Handle<ObjectTemplate>init_async();

    Handle<ObjectTemplate>o = ObjectTemplate::New();
    OSETO(o, console, init_console());
    OSETO(o, process, init_process());
    OSETO(o, v8, init_v8());
    OSETO(o, mem, init_mem());
    OSETO(o, pthread, init_pthread());
    OSETO(o, fs, init_fs());
    OSETO(o, net, init_net());
    OSETO(o, async, init_async());
    return o;
}

static const char* ToCString (const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
}

void ReportException (v8::TryCatch* try_catch) {
    v8::HandleScope handle_scope;
    v8::String::Utf8Value exception(try_catch->Exception());
    const char* exception_string = ToCString(exception);
    v8::Handle<v8::Message> message = try_catch->Message();
    if (message.IsEmpty()) {
        // V8 didn't provide any extra information about this error; just
        // print the exception.
        printf("%s\n", exception_string);
    }
    else {
        // Print (filename):(line number): (message).
        v8::String::Utf8Value filename(message->GetScriptResourceName());
        const char* filename_string = ToCString(filename);
        int linenum = message->GetLineNumber();
        printf("%s:%i: %s\n", filename_string, linenum, exception_string);
        // Print line of source code.
        v8::String::Utf8Value sourceline(message->GetSourceLine());
        const char* sourceline_string = ToCString(sourceline);
        printf("%s\n", sourceline_string);
        // Print wavy underline (GetUnderline is deprecated).
        int start = message->GetStartColumn();
        for (int i = 0; i < start; i++) {
            printf(" ");
        }
        int end = message->GetEndColumn();
        for (int i = start; i < end; i++) {
            printf("^");
        }
        printf("\n");
        v8::String::Utf8Value stack_trace(try_catch->StackTrace());
        if (stack_trace.length() > 0) {
            const char* stack_trace_string = ToCString(stack_trace);
            printf("%s\n", stack_trace_string);
        }
        else {
            printf("no stack trace available\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: vt file.js\n");
        exit(1);
    }
    char *startup = readFile(argv[1]);
    if (!*startup) {
        printf("%s not found\n", argv[1]);
    }

    // Get the default Isolate created at startup.
    ISOLATE = Isolate::GetCurrent();
    LOCK;

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(ISOLATE);

    Handle<ObjectTemplate>global = ObjectTemplate::New();
    OSETFN(global, timer);
    OSETFN(global, log);
    OSETFN(global, include);
    OSETO(global, builtin, builtins());

    // Create a new context.
    CONTEXT = Context::New(ISOLATE, NULL, global);

    // Enter the created context for compiling and
    // running the hello world script. 
    Context::Scope context_scope(CONTEXT);
    {
        LOCK;
        Locker::StartPreemption(10000);
        TryCatch tryCatch;
        Handle<Script>init = Script::New(String::New("global=this; module = {}; include('builtin/all.js');"), String::New("builtin"));
        init->Run();
        Handle<Script>script = Script::Compile(String::New(startup), String::New(argv[1]));
        if (script.IsEmpty()) {
            ReportException(&tryCatch);
            exit(1);
        }
        Handle<Value>v = script->Run();
        if (v.IsEmpty()) {
           ReportException(&tryCatch);
           exit(1);
        }

        Handle<String> process_name = String::New("main");
        Handle<Value> process_val = context->Global()->Get(process_name);
        if (!process_val.IsEmpty() && process_val->IsFunction()) {
            Handle<Function> process_fun = Handle<Function>::Cast(process_val);
            Persistent<Function>mainFunc = Persistent<Function>::New(ISOLATE, process_fun);
            int ac = argc - 2;
            if (ac < 0) {
                ac = 0;
            }
            Handle<Value>av[ac];
            for (int i = 2; i < argc; i++) {
                av[i - 2] = String::New(argv[i]);
            }
            v = mainFunc->Call(context->Global(), ac, av);
            if (v.IsEmpty()) {
                ReportException(&tryCatch);
                exit(1);
            }
            if (v->IsInt32()) {
                exit(v->IntegerValue());
            }
        }
    }
    return 0;
}
