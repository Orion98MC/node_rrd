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
    time_t last;
};

}

static void async_worker(uv_work_t *req);
static void async_after(uv_work_t *req);

NAN_METHOD(last) { // rrd.last(String filename, Function callback);
    Nan::HandleScope scope;

    CHECK_FUN_ARG(1)

    // Create info baton
    CREATE_ASYNC_BATON(Infos, _info)

    // Get filename
    SET_CHARS_ARG(0, _info->filename)

    // Get callback
    SET_PERSFUN_ARG(1, _info->callback)

    uv_queue_work(uv_default_loop(), &_info->request, async_worker, (uv_after_work_cb)async_after);

    return;
}

static void async_worker(uv_work_t *req) {
    auto info = static_cast<Infos*>(req->data);

    info->last = rrd_last_r(info->filename);
}

static void async_after(uv_work_t *req) {
    Nan::HandleScope scope;

    auto info = static_cast<Infos*>(req->data);

    Local<Value> res[] = { Nan::New<Number>(info->last) };
    info->callback->Call(1, res);

    delete(info);
}

}
