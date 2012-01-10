#pragma comment(lib, "node")

#include <node.h>
#include <v8.h>
#include <string>

#include "tsqlftwHelper.h"

using namespace node;
using namespace v8;

class tsqlftw: ObjectWrap
{
private:
    tsqlftwHelper* _tsqlftwHelper;
public:

    static Persistent<FunctionTemplate> s_ct;
    static void NODE_EXTERN Init(Handle<Object> target)
    {
        HandleScope scope;

        // set the constructor function
        Local<FunctionTemplate> t = FunctionTemplate::New(New);

        // set the node.js/v8 class name
        s_ct = Persistent<FunctionTemplate>::New(t);
        s_ct->InstanceTemplate()->SetInternalFieldCount(1);
        s_ct->SetClassName(String::NewSymbol("tsqlftw"));

        // registers a class member functions 
        NODE_SET_PROTOTYPE_METHOD(s_ct, "connect", connect);
		NODE_SET_PROTOTYPE_METHOD(s_ct, "query", query);
		NODE_SET_PROTOTYPE_METHOD(s_ct, "close", close);

        target->Set(String::NewSymbol("tsqlftw"),
            s_ct->GetFunction());
    }

    tsqlftw() 
    {
        _tsqlftwHelper = tsqlftwHelper::New();
    }

    ~tsqlftw()
    {
        delete _tsqlftwHelper;
    }

    static Handle<Value> New(const Arguments& args)
    {
        HandleScope scope;
        tsqlftw* pm = new tsqlftw();
        pm->Wrap(args.This());
        return args.This();
    }

    struct BatonConnect {
        uv_work_t request;
        tsqlftwHelper* tsqlftwHelper;
        Persistent<Function> callback;
		std::string connString;
        bool error;
        std::string error_message;
        std::string result;
    };

	struct BatonQuery {
        uv_work_t request;
        tsqlftwHelper* tsqlftwHelper;
        Persistent<Function> callback;
		std::string query;
        bool error;
        std::string error_message;
        std::string result;
    };

	struct BatonClose {
        uv_work_t request;
        tsqlftwHelper* tsqlftwHelper;
        Persistent<Function> callback;
        bool error;
        std::string error_message;
        std::string result;
    };

    static Handle<Value> connect(const Arguments& args)
    {
        HandleScope scope;

        if (!args[0]->IsString()) {
            return ThrowException(Exception::TypeError(
                String::New("First argument must be a string")));
        }

        if (!args[1]->IsFunction()) {
            return ThrowException(Exception::TypeError(
                String::New("Second argument must be a callback function")));
        }

        Local<String> connString = Local<String>::Cast(args[0]);
        // There's no ToFunction(), use a Cast instead.
        Local<Function> callback = Local<Function>::Cast(args[1]);

        tsqlftw* so = ObjectWrap::Unwrap<tsqlftw>(args.This());

        // create a state object
        BatonConnect* baton = new BatonConnect();
        baton->request.data = baton;
        baton->tsqlftwHelper = so->_tsqlftwHelper;
        baton->callback = Persistent<Function>::New(callback);
        baton->connString = *v8::String::AsciiValue(connString);

        // register a worker thread request
        uv_queue_work(uv_default_loop(), &baton->request,
            StartConnect, AfterConnect);

        return Undefined();

    }

    static Handle<Value> query(const Arguments& args)
    {
        HandleScope scope;

        if (!args[0]->IsString()) {
            return ThrowException(Exception::TypeError(
                String::New("First argument must be a string")));
        }

        if (!args[1]->IsFunction()) {
            return ThrowException(Exception::TypeError(
                String::New("Second argument must be a callback function")));
        }

        Local<String> query = Local<String>::Cast(args[0]);
        // There's no ToFunction(), use a Cast instead.
        Local<Function> callback = Local<Function>::Cast(args[1]);

        tsqlftw* so = ObjectWrap::Unwrap<tsqlftw>(args.This());

        // create a state object
        BatonQuery* baton = new BatonQuery();
        baton->request.data = baton;
        baton->tsqlftwHelper = so->_tsqlftwHelper;
        baton->callback = Persistent<Function>::New(callback);
        baton->query = *v8::String::AsciiValue(query);

        // register a worker thread request
        uv_queue_work(uv_default_loop(), &baton->request,
            StartQuery, AfterQuery);

        return Undefined();
    }

	static Handle<Value> close(const Arguments& args)
    {
        HandleScope scope;

        if (!args[0]->IsFunction()) {
            return ThrowException(Exception::TypeError(
                String::New("First argument must be a callback function")));
        }

        Local<Function> callback = Local<Function>::Cast(args[0]);

        tsqlftw* so = ObjectWrap::Unwrap<tsqlftw>(args.This());

        // create a state object
        BatonClose* baton = new BatonClose();
        baton->request.data = baton;
        baton->tsqlftwHelper = so->_tsqlftwHelper;
        baton->callback = Persistent<Function>::New(callback);

        // register a worker thread request
        uv_queue_work(uv_default_loop(), &baton->request,
            StartClose, AfterClose);

        return Undefined();
    }

    static void StartConnect(uv_work_t* req)
    {
        BatonConnect *baton = static_cast<BatonConnect*>(req->data);
        baton->error = baton->tsqlftwHelper->connect(baton->connString, baton->error_message, baton->result);
    }

    static void AfterConnect(uv_work_t *req)
    {
        HandleScope scope;
        BatonConnect* baton = static_cast<BatonConnect*>(req->data);

        if (baton->error) 
        {
            Local<Value> err = Exception::Error(
                String::New(baton->error_message.c_str()));
            Local<Value> argv[] = { err };

            TryCatch try_catch;
            baton->callback->Call(
                Context::GetCurrent()->Global(), 1, argv);

            if (try_catch.HasCaught()) {
                node::FatalException(try_catch);
            }        
        } 
        else 
        {
            const unsigned argc = 2;
            Local<Value> argv[argc] = {
                Local<Value>::New(Null()),
                Local<Value>::New(String::New(baton->result.c_str()))
            };

            TryCatch try_catch;
            baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

            if (try_catch.HasCaught()) {
                FatalException(try_catch);
            }
        }

        baton->callback.Dispose();
        delete baton;
    }

    static void StartQuery(uv_work_t* req)
    {
        BatonQuery *baton = static_cast<BatonQuery*>(req->data);
        baton->error = baton->tsqlftwHelper->query(baton->query, baton->error_message, baton->result);
    }

    static void AfterQuery(uv_work_t *req)
    {
        HandleScope scope;
        BatonQuery* baton = static_cast<BatonQuery*>(req->data);

        if (baton->error) 
        {
            Local<Value> err = Exception::Error(
                String::New(baton->error_message.c_str()));
            Local<Value> argv[] = { err };

            TryCatch try_catch;
            baton->callback->Call(
                Context::GetCurrent()->Global(), 1, argv);

            if (try_catch.HasCaught()) {
                node::FatalException(try_catch);
            }        
        } 
        else 
        {
            const unsigned argc = 2;
            Local<Value> argv[argc] = {
                Local<Value>::New(Null()),
                Local<Value>::New(String::New(baton->result.c_str()))
            };

            TryCatch try_catch;
            baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

            if (try_catch.HasCaught()) {
                FatalException(try_catch);
            }
        }

        baton->callback.Dispose();
        delete baton;
    }

    static void StartClose(uv_work_t* req)
    {
        BatonClose *baton = static_cast<BatonClose*>(req->data);
        baton->error = baton->tsqlftwHelper->close(baton->error_message, baton->result);
    }

    static void AfterClose(uv_work_t *req)
    {
        HandleScope scope;
        BatonClose* baton = static_cast<BatonClose*>(req->data);

        if (baton->error) 
        {
            Local<Value> err = Exception::Error(
                String::New(baton->error_message.c_str()));
            Local<Value> argv[] = { err };

            TryCatch try_catch;
            baton->callback->Call(
                Context::GetCurrent()->Global(), 1, argv);

            if (try_catch.HasCaught()) {
                node::FatalException(try_catch);
            }        
        } 
        else 
        {
            const unsigned argc = 2;
            Local<Value> argv[argc] = {
                Local<Value>::New(Null()),
                Local<Value>::New(String::New(baton->result.c_str()))
            };

            TryCatch try_catch;
            baton->callback->Call(Context::GetCurrent()->Global(), argc, argv);

            if (try_catch.HasCaught()) {
                FatalException(try_catch);
            }
        }

        baton->callback.Dispose();
        delete baton;
    }

};

Persistent<FunctionTemplate> tsqlftw::s_ct;

extern "C" {
    void NODE_EXTERN init (Handle<Object> target)
    {
        tsqlftw::Init(target);
        LoadAssembly();
    }
    NODE_MODULE(tsqlftw, init);
}