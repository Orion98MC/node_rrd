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
    char *cf;
    time_t start;
    time_t end;        /* which time frame do you want ?
                         * will be changed to represent reality */
    unsigned long step;    /* which stepsize do you want?
                             * will be changed to represent reality */
    unsigned long ds_cnt;  /* number of data sources in file */
    char **ds_namv;    /* names of data_sources */
    rrd_value_t *data;

    ~Infos();
};

Infos::~Infos() {
    free(cf);
    for (unsigned long i = 0; i < ds_cnt; i++) free(ds_namv[i]);
    free(ds_namv);
    free(data);
}

}

static void async_worker(uv_work_t *req);
static void async_after(uv_work_t *req);

NAN_METHOD(fetch) { // rrd.fetch(String filename, String cf, Number start, Number end, Number step, Function callback);
    Nan::HandleScope scope;

    CHECK_FUN_ARG(5)

    // Create baton
    CREATE_ASYNC_BATON(Infos, _info)

    // Get filename
    SET_CHARS_ARG(0, _info->filename)

    // Get template string
    SET_CHARS_ARG(1, _info->cf);

    _info->start = Nan::To<uint32_t>(info[2]).FromJust();
    _info->end = Nan::To<uint32_t>(info[3]).FromJust();
    _info->step = Nan::To<uint32_t>(info[4]).FromJust();

    // Get callback
    SET_PERSFUN_ARG(5, _info->callback)

    uv_queue_work(uv_default_loop(), &_info->request, async_worker, (uv_after_work_cb)async_after);

    return;
}

static void async_worker(uv_work_t *req) {
    auto info = static_cast<Infos*>(req->data);

    info->status = rrd_fetch_r(
        (const char *)info->filename,
        (const char *)info->cf,
        &info->start,
        &info->end,
        &info->step,
        &info->ds_cnt,
        &info->ds_namv,
        &info->data
    );
}

Local<Object> current_data_to_object(unsigned long ds_cnt, char ** ds_namv, rrd_value_t *data);

static void async_after(uv_work_t *req) {
    Nan::HandleScope scope;

    auto info = static_cast<Infos*>(req->data);

    if (info->status == 0) {
        rrd_value_t *datai;
        long ti;

        datai = info->data;
        for (ti = info->start + info->step; ti <= info->end; ti += info->step) {
            Local<Value> argv[] = { Nan::New<Number>(ti), current_data_to_object(info->ds_cnt, info->ds_namv, datai) };
            info->callback->Call(2, argv);
            datai += info->ds_cnt;
        }

        /* Last callback with (null, null) */
        Local<Value> argv[] = { Nan::Null(), Nan::Null() };
        info->callback->Call(2, argv);

    } else {
        Local<Value> res[] = { Nan::New<Number>(info->status) };
        info->callback->Call(1, res);
    }

    delete(info);
}

/* @return
    {
        <ds_name1>: <value1>,
        <ds_name2>: <value2>,
        ...
    }
*/
Local<Object> current_data_to_object(unsigned long ds_cnt, char ** ds_namv, rrd_value_t *data) {
    Nan::EscapableHandleScope scope;

    auto obj = ObjectTemplate::New();
    auto result = obj->NewInstance();

    unsigned long ii;
    rrd_value_t *datai;

    datai = data;
    for (ii = 0; ii < ds_cnt; ii++) {
        result->Set(Nan::New<String>(ds_namv[ii]).ToLocalChecked(), Nan::New<Number>(datai[ii]));
    }

    return scope.Escape(result);
}

}
