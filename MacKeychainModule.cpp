#include <node.h>
#include <v8.h>
#include "KeychainUtils.h"

using namespace v8;
using namespace std;

/**
 * in javascript: module.getAccountsForHost(hostname)
 */
void getAccountsForHost(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.Length() < 1) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Missing host argument")));
        return;
    }

    String::Utf8Value param1(args[0]->ToString());
    std::string host(*param1);

    vector<KeychainUtils::InternetRecord>* recs = KeychainUtils::getInternetRecordsForService(host.c_str());

    if (recs) {

        size_t recsCount = recs->size();
        Handle<Array> array = Array::New(isolate, recsCount);

        if (recsCount > 0) {
            int i = 0;
            for(vector<KeychainUtils::InternetRecord>::iterator it = recs->begin(); it != recs->end(); ++it) {
                KeychainUtils::InternetRecord rec =  *it;
                Handle<Object> recObj = Object::New(isolate);

                recObj->Set(String::NewFromUtf8(isolate, "username"), String::NewFromUtf8(isolate, rec.username.c_str()));
                recObj->Set(String::NewFromUtf8(isolate, "server"), String::NewFromUtf8(isolate, rec.server.c_str()));
                recObj->Set(String::NewFromUtf8(isolate, "protocol"), String::NewFromUtf8(isolate, rec.protocol.c_str()));
                recObj->Set(String::NewFromUtf8(isolate, "path"), String::NewFromUtf8(isolate, rec.path.c_str()));
                recObj->Set(String::NewFromUtf8(isolate, "port"), Integer::New(isolate, rec.port));
                recObj->Set(String::NewFromUtf8(isolate, "ref"), String::NewFromUtf8(isolate, rec.persistentRef.c_str()));

                array->Set(i++, recObj);
            }
        }

        delete recs;

        args.GetReturnValue().Set(array);
    } else {
        args.GetReturnValue().Set(Array::New(isolate, 0));
    }
}

/**
 * in javascript: module.getPasswordForRef(ref)
 */
void getPasswordForRef(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    HandleScope scope(isolate);

    if (args.Length() < 1) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Missing ref argument")));
        return;
    }

    String::Utf8Value param1(args[0]->ToString());
    std::string persistentRefB64(*param1);

    std::string* password = KeychainUtils::getPasswordForItem(persistentRefB64);
    if (password) {
        args.GetReturnValue().Set(String::NewFromUtf8(isolate, password->c_str()));
        delete password;
    } else {
        args.GetReturnValue().Set(Null(isolate));
    }
}

void init(Handle<Object> target) {
    NODE_SET_METHOD(target, "getAccountsForHost", getAccountsForHost);
    NODE_SET_METHOD(target, "getPasswordForRef", getPasswordForRef);
}

NODE_MODULE(binding, init);
