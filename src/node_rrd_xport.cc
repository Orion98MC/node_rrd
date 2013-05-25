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
    int argc;
    char **argv;
    
    time_t start;
    time_t end;        /* which time frame do you want ?
                         * will be changed to represent reality */
    int xsize; 
    unsigned long step;    /* which stepsize do you want? 
                             * will be changed to represent reality */
    unsigned long col_cnt; /* number of data columns in the result */
    char **legend_v;   /* legend entries */
    rrd_value_t *data; /* two dimensional array containing the data */

    ~Infos();
};

Infos::~Infos() {
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
    for (unsigned long i = 0; i < col_cnt; i++) free(legend_v[i]);
    free(legend_v);
    free(data);
}

}

static void async_worker(uv_work_t *req);
static void async_after(uv_work_t *req);

Handle<Value> xport(const Arguments &args) { // rrd.xport(Array xport_arguments, Function callback);
    HandleScope scope;

    CHECK_FUN_ARG(1)

    // Create baton
    CREATE_ASYNC_BATON(Infos, info)
      
    // Get xport_arguments
    SET_ARGC_ARGV_ARG(0, info->argc, info->argv)
      
    // Get callback
    SET_PERSFUN_ARG(1, info->callback)

    uv_queue_work(uv_default_loop(), &info->request, async_worker, async_after);

    return Undefined();
}

static void async_worker(uv_work_t *req) {
    Infos * info = static_cast<Infos*>(req->data);
    
    info->status = rrd_xport(
      info->argc, 
      info->argv, 
      &info->xsize, 
      &info->start, 
      &info->end, 
      &info->step, 
      &info->col_cnt, 
      &info->legend_v, 
      &info->data
    );
}

static void async_after(uv_work_t *req) {
    HandleScope scope;

    Infos * info = static_cast<Infos*>(req->data);
    
    if (info->status == 0) {
      rrd_value_t *datai;
        long ti;

        datai = info->data;
        for (ti = info->start + info->step; ti <= info->end; ti += info->step) {
            Handle<Value> argv[] = { Number::New(ti), current_data_to_object(info->col_cnt, info->legend_v, datai) };
            info->callback->Call(Context::GetCurrent()->Global(), 2, argv);
            datai += info->col_cnt;
        }
        
        /* Last callback with (null, null) */
        Handle<Value> argv[] = { Null(), Null() };
        info->callback->Call(Context::GetCurrent()->Global(), 2, argv);
                
        
    } else {
        Handle<Value> res[] = { Number::New(info->status) };
        info->callback->Call(Context::GetCurrent()->Global(), 1, res);
    }

    delete(info);
}

}
