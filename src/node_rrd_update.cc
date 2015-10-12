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
    char * tmplt;
    int argc;
    char **argv;

    ~Infos();
};

Infos::~Infos() {
    free(tmplt);
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
}

}

static void async_worker(uv_work_t *req);
static void async_after(uv_work_t *req);

NAN_METHOD(update) { // rrd.update(String filename, String tmplt, Array updates, Function callback);
    Nan::HandleScope scope;

    CHECK_FUN_ARG(3)

    // Create info baton
    CREATE_ASYNC_BATON(Infos, _info)

    // Get filename
    SET_CHARS_ARG(0, _info->filename)

    // Get template string
    SET_CHARS_ARG(1, _info->tmplt);

    // Get updates array
    SET_ARGC_ARGV_ARG(2, _info->argc, _info->argv)

    // Get callback
    SET_PERSFUN_ARG(3, _info->callback)

    uv_queue_work(uv_default_loop(), &_info->request, async_worker, (uv_after_work_cb)async_after);

    return;
}

static void async_worker(uv_work_t *req) {
    auto info = static_cast<Infos*>(req->data);

    int status = rrd_update_r(
        (const char *)info->filename,
        (const char *)info->tmplt,
        info->argc,
        (const char **)info->argv
    );

    info->status = status;
}

static void async_after(uv_work_t *req) {
    Nan::HandleScope scope;

    auto info = static_cast<Infos*>(req->data);

    Local<Value> res[] = { Nan::Null() };
    if (info->status < 0) res[0] = Nan::New<String>(rrd_get_error()).ToLocalChecked();
    info->callback->Call(1, res);

    rrd_clear_error();

    delete(info);
}

}
