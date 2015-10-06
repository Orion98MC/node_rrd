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

#include <v8.h>
#include <node.h>
#include <node_buffer.h>
#include <stdlib.h> // malloc ...
#include <uv.h>
#include <rrd.h>
#include <math.h> // isnan ...

#ifdef DEBUG
#   include <iostream> 
#endif

#include <nan.h>

using namespace v8;
using namespace node;

namespace node_rrd {

/* The async baton used by libuv */
class AsyncInfos {
public:
    uv_work_t request; 

    /* Used in the uv work callback */
    char *filename;
    int status;
    
    /* Used by the uv after_work callback */
    Nan::Callback *callback;

    ~AsyncInfos() { delete callback; free(filename); }
};

/* Some useful macros to extract arguments */
#define SET_CHARS_ARG(I, VAR)                                           \
    String::Utf8Value _chars ## I(info[I]->ToString());                 \
    VAR = strndup(*_chars ## I, _chars ## I.length());

#define SET_ARGC_ARGV_ARG(I, VAR_ARGC, VAR_ARGV)                        \
    Local<Array> argv ## I = Local<Array>::Cast(info[I]);               \
    VAR_ARGC = argv ## I->Length();                                     \
    VAR_ARGV = (char**)malloc(sizeof(char*) * VAR_ARGC);                \
    for (int i = 0; i < VAR_ARGC; i++) {                                \
        String::Utf8Value arg(argv ## I->Get(i));                       \
        VAR_ARGV[i] = strndup(*arg, arg.length());                      \
    }

#define CHECK_FUN_ARG(I)                                                \
  if (info.Length() <= (I) || !info[I]->IsFunction())                   \
    return Nan::ThrowTypeError("Argument " #I " must be a function");

#define SET_PERSFUN_ARG(I, VAR)                                         \
  VAR = new Nan::Callback(Local<Function>::Cast(info[I]));

#define CREATE_ASYNC_BATON(K, VAR)                                      \
  K *VAR = new K();                                                     \
  VAR->request.data = VAR;

/*
    create
    Create an empty RRD

    Usage: 
        rrd.create(String filename, Number step, Number start, Array spec, Function callback);

    Example:
        var rrd = require('rrd');

        var now = Math.ceil((new Date).getTime() / 1000);
        rrd.create('/tmp/test.rrd', 60, now, ["DS:busy:GAUGE:120:0:U", "RRA:LAST:0.5:1:60"], function (error) { 
            if (error === null) console.log("Created!");
            else console.log("Error:", error);
        });
*/
NAN_METHOD(create);



/*
    update
    Update an RRD

    Usage: 
        rrd.update(String filename, String tmplt, Array updates, Function callback);

    Example:
        var rrd = require('rrd');

        var value = 80.0;
        var now = Math.ceil((new Date).getTime() / 1000);

        rrd.update('/tmp/test.rrd', 'busy', [[now, value].join(':')], function (error) { 
            if (error === null) console.log("Updated!");
            else console.log("Error:", error);
        });
*/
NAN_METHOD(update);



/*
    fetch
    Fetch data in an RRD

    Usage: 
        rrd.fetch(String filename, String cf, Number start, Number end, Number step, Function callback);

    Example:
        var rrd = require('rrd');

        var now = Math.ceil((new Date).getTime() / 1000); 
        rrd.fetch("/tmp/test.rrd", "LAST", now - 1000, now, null, function (time, data) {
            console.log(time, data);
        });
*/
NAN_METHOD(fetch);



/*
    last
    Get database last update time

    Usage: 
        rrd.last(String filename, Function callback);

    Example:
        var rrd = require('rrd');

        rrd.last('/tmp/test.rrd', function (time) { 
            console.log('Last updated on', new Date(time * 1000));
        });
*/
NAN_METHOD(last);



/*
    info
    Get database info

    Usage: 
        rrd.info(String filename, Function callback);

    Example:
        var rrd = require('rrd');

        rrd.info('/tmp/test.rrd', function (info) { 
            console.log(info);
        });
*/
NAN_METHOD(info);

}

using namespace node_rrd;
