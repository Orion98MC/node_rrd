/*
    RRDtool (http://oss.oetiker.ch/rrdtool/) bindings module for node (http://nodejs.org)
    
    Copyright (c), 2012 Thierry Passeron

    The MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to
    deal in the Software without restriction, including without limitation the
    rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
    sell copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
    IN THE SOFTWARE.
*/

#include "node_rrd.h"

namespace node_rrd {

namespace {

class Infos: public AsyncInfos {
public:
    rrd_info_t *data;

    ~Infos() { free(data); }
};

}

static void async_worker(uv_work_t *req);
static void async_after(uv_work_t *req);

Handle<Value> info(const Arguments &args) { // rrd.info(String filename, Function callback);
    HandleScope scope;

    CHECK_FUN_ARG(1)

    // Create info baton
    CREATE_ASYNC_BATON(Infos, info)

    // Get filename
    SET_CHARS_ARG(0, info->filename)

    // Get callback
    SET_PERSFUN_ARG(1, info->callback)

    uv_queue_work(uv_default_loop(), &info->request, async_worker, (uv_after_work_cb)async_after);

    return Undefined();
}

static void async_worker(uv_work_t *req) {
    Infos * info = static_cast<Infos*>(req->data);

    info->data = rrd_info_r(info->filename);
}

static Handle<Object>info_data_to_object(rrd_info_t *data);

static void async_after(uv_work_t *req) {
    HandleScope scope;

    Infos * info = static_cast<Infos*>(req->data);
    
    Handle<Value> res[] = { info_data_to_object((rrd_info_t *)info->data) };
    info->callback->Call(Context::GetCurrent()->Global(), 1, res);

    delete(info);    
}

/*
 * Based off of http://sambro.is-super-awesome.com/2011/03/03/creating-a-proper-buffer-in-a-node-c-addon/
 */
static Local<Object> makeBuffer(char* data, size_t size) {
  HandleScope scope;

  Local<Buffer> slowBuffer = Buffer::New(data, size);
  Local<Object> global = Context::GetCurrent()->Global();
  Local<Value> bv = global->Get(String::NewSymbol("Buffer"));
  assert(bv->IsFunction());
  Local<Function> b = Local<Function>::Cast(bv);
  Handle<Value> argv[3] = { slowBuffer->handle_, Integer::New(size), Integer::New(0) };
  Local<Object> fastBuffer = b->NewInstance(3, argv);

  return scope.Close(fastBuffer);
}

static Handle<Object>info_data_to_object(rrd_info_t *data) {
    HandleScope scope;

    Handle<ObjectTemplate> info = ObjectTemplate::New();
    Handle<Object> instance = info->NewInstance();
    
    while (data) {
         v8::Handle<v8::String> key = String::New(data->key);

        switch (data->type) {
        case RD_I_VAL:
            instance->Set(key, Number::New(data->value.u_val)); // if data->value.u_val is not a number, it will be NaN
            break;
        case RD_I_CNT:
            instance->Set(key, Number::New(data->value.u_cnt));
            break;
        case RD_I_INT:
            instance->Set(key, Number::New(data->value.u_int));
            break;
        case RD_I_STR:
            instance->Set(key, String::New(data->value.u_str));
            break;
        case RD_I_BLO:
            instance->Set(key, makeBuffer((char*)data->value.u_blo.ptr, data->value.u_blo.size));
            // fwrite(data->value.u_blo.ptr, data->value.u_blo.size, 1, stdout);
            break;
        }
        data = data->next;
    }

    return scope.Close(instance);
}

}
