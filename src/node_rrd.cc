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


#pragma mark tool methods

/* @return
    {
        <ds_name1>: <value1>,
        <ds_name2>: <value2>,
        ...
    }
*/
Handle<Object> current_data_to_object(unsigned long ds_cnt, char ** ds_namv, rrd_value_t *data) { 
    HandleScope scope;

    Handle<ObjectTemplate> obj = ObjectTemplate::New();
    Handle<Object> result = obj->NewInstance();

    unsigned long ii;
    rrd_value_t *datai;

    datai = data;
    for (ii = 0; ii < ds_cnt; ii++) {
        result->Set(String::New(ds_namv[ii]), Number::New(datai[ii]));
    }

    return scope.Close(result);
}

#pragma mark -



extern "C" {
    static void init(Handle<Object> target) {
        HandleScope scope;
        NODE_SET_METHOD(target, "create", node_rrd::create);
        NODE_SET_METHOD(target, "update", node_rrd::update);
        NODE_SET_METHOD(target, "fetch", node_rrd::fetch);
        NODE_SET_METHOD(target, "last", node_rrd::last);
        NODE_SET_METHOD(target, "info", node_rrd::info);
        NODE_SET_METHOD(target, "xport", node_rrd::xport);
    }

    NODE_MODULE(rrd_bindings, init)
}
