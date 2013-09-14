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
    unsigned long step;
    time_t time;
    int argc;
    char **argv;

    ~Infos();
};

Infos::~Infos() {
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
}

}

static void async_worker(uv_work_t *req);
static void async_after(uv_work_t *req);

Handle<Value> create(const Arguments &args) { /* rrd.create(String filename, Number step, Number start, Array spec, Function callback); */
    HandleScope scope;

    CHECK_FUN_ARG(4)

    // Create info baton
    CREATE_ASYNC_BATON(Infos, info)

    // Get filename
    SET_CHARS_ARG(0, info->filename)

    // Get step
    info->step = args[1]->Uint32Value();

    // Get start time
    info->time = args[2]->Uint32Value();

    // Get spec array
    SET_ARGC_ARGV_ARG(3, info->argc, info->argv)

    // Get callback
    SET_PERSFUN_ARG(4, info->callback)

    uv_queue_work(uv_default_loop(), &info->request, async_worker, (uv_after_work_cb)async_after);

    return Undefined();
}

static void async_worker(uv_work_t *req) {
    Infos * info = static_cast<Infos*>(req->data);
    
    info->status = rrd_create_r(
        (const char*)info->filename,
        info->step,
        info->time,
        info->argc,
        (const char**)info->argv
    );
}

static void async_after(uv_work_t *req) {
    HandleScope scope;

    Infos * info = static_cast<Infos*>(req->data);
    
    Handle<Value> res[] = { info->status < 0 ? String::New(rrd_get_error()) : Null() };
    info->callback->Call(Context::GetCurrent()->Global(), 1, res);

    rrd_clear_error();
    
    delete(info);
}

}
