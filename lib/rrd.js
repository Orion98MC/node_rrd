/*
    RRD (http://oss.oetiker.ch/rrdtool/) bindings module for node (http://nodejs.org)
    
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

var bindings = require('bindings')('rrd_bindings')
,   conform = require('./argue').conform
,   fs = require('fs')
;


/* 
    Create an RRD

    rrd.create("/tmp/test.rrd", 60, now(), ["DS:busy:GAUGE:120:0:U", "RRA:LAST:0.5:1:60"], function (status) { ... });

    _OR_ (TO BE IMPLEMENTED)

    rrd.create("/tmp/test.rrd", {
          step: 60
        , start: now()
        , ds: {
            'load1' : {
                  type: 'GAUGE'
                , heartbeat: 120
                , min: 0
                , max: undefined
            }
        }
        , rra: [
            {
                'cf': 'LAST'
                , xff: 0.5
                , steps: 1
                , rows: 60
            }
        ]
    }, function (status) { ... });

*/
var create = function () {
    if (conform.apply([String, Number, Number, Array, Function], arguments)) {
        return bindings.create.apply(null, arguments);
    } else {
        throw "Arguments' format not implemented!";
    }
};

var update = function () {
    if (conform.apply([String, String, Array, Function], arguments)) {
        return bindings.update.apply(null, arguments);
    } else {
        throw "Arguments' format not implemented!";
    }
};

var fetch = function () {
    if (conform.apply([String, String, Number, Number, Number, Function], arguments)) {
        return bindings.fetch.apply(null, arguments);
    } else {
        throw "Arguments' format not implemented!";
    }
};

var last = function () {
    return bindings.last.apply(null, arguments);
};

var info = function () {
    return bindings.info.apply(null, arguments);
};

var RRD = {
      core: bindings
    , create: create
    , update: update
    , fetch: fetch
    , last: last
    , info: info
}


module.exports = RRD;
