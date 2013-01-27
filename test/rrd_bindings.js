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

var
	assert = require('assert')
,	rrd_bindings = null
,	filename = "/tmp/mocha-test.rrd"
,	fs = require('fs')
;

function createAnRRD(done) {
	rrd_bindings.create(filename, 60, now() - 60, [
		  "DS:x:GAUGE:60:0:U"
		, "RRA:LAST:0.5:1:60"
	], function(status) { 
		assert(status === null); 
		done(status === null ? undefined : status); 
	});
}

function updateRRDWith(data, done) {
	rrd_bindings.update(filename, "x", data, function(status) { 
		assert(status === null); 
		done(status === null ? undefined : status); 
	});
}

function now() { return Math.ceil((new Date).getTime() / 1000); }




describe('Native bindings', function(){
  
    it('should load rrd_bindings', function () {
    	rrd_bindings = require('bindings')('rrd_bindings');
    	assert(typeof rrd_bindings === 'object');
    });

    describe('Create', function () {
    	it('should have a create function', function () {
    		assert(typeof rrd_bindings.create === 'function');
    	})

    	it('should create', function (done) {
    		createAnRRD(done);
    	});

    });

    describe('Update', function () {
    	it('should have a update function', function () {
    		assert(typeof rrd_bindings.update === 'function');
    	})

    	it('should update', function (done) {
    		var data = [now(), "10"].join(':');
    		updateRRDWith([ data ], done);
    	});
    });

	describe('Fetch', function () {
    	it('should have a fetch function', function () {
    		assert(typeof rrd_bindings.fetch === 'function');
    	});

    	it('should fetch', function (done) {
    		
    		rrd_bindings.fetch(filename, 'LAST', now()-3600, now(), null, function (time, data) {
          if (time === null && data === null) done();
    		});
        
    	});
    });

	describe('Last', function () {
    	it('should have a last function', function () {
    		assert(typeof rrd_bindings.last === 'function');
    	});

    	it('should get last update time', function (done) {
    		rrd_bindings.last(filename, function (time) {
    			assert(time > 0);
    			done();
    		});
    	});
    });    

	describe('Info', function () {
    	it('should have a info function', function () {
    		assert(typeof rrd_bindings.info === 'function');
    	});

    	it('should get RRD info', function (done) {
    		rrd_bindings.info(filename, function (data) {
    			assert(typeof data === 'object');
    			assert(data.filename === filename);
    			done();
    		});
    	});
    });        
    
})

// Remove the database after all
after(function() {
	if (fs.existsSync(filename)) fs.unlinkSync(filename);
});
