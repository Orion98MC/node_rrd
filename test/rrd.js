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
, whenDone = require('./whenDone')
, fs = require('fs')
,	rrd = null
;

describe('rrd', function(){
  
    it('should load rrd', function () {
    	rrd = require('../lib/rrd');
    	assert(typeof rrd === 'object');
    })

    it('should have .create function', function () {
    	assert(typeof rrd.create === 'function');
    });

    it('should have .update function', function () {
    	assert(typeof rrd.update === 'function');
    });

    it('should have .info function', function () {
    	assert(typeof rrd.info === 'function');
    });

    it('should have .last function', function () {
    	assert(typeof rrd.last === 'function');
    });

    it('should have .fetch function', function () {
    	assert(typeof rrd.fetch === 'function');
    });
  
});

describe('rrd DS', function(){
  
  rrd = require('../lib/rrd');
  
  assert(typeof rrd === 'object');
  var DS = rrd.DS;
  
  it('should create a DS with rrd.DS', function () {	
  	assert(typeof rrd.DS === 'function');
    
    var ds = new DS({name: "probe", type: "GAUGE", heartbeat: 60, min: 0, max: "U"});
    assert(ds);
    
  });

  it('should output DS spec', function () {	
    
    var ds = new DS({name: "probe", type: "gauge", heartbeat: 60, min: 0, max: "U"});
    assert(ds.toString());
    assert(ds.toString() === "DS:probe:GAUGE:60:0:U");
    
  });  
  
});


describe('rrd RRA', function(){
  
  rrd = require('../lib/rrd');
  
  assert(typeof rrd === 'object');
  var RRA = rrd.RRA;
  
  it('should create a RRA with rrd.RRA', function () {	
  	assert(typeof rrd.RRA === 'function');
    
    var rra = new RRA({cf:"LAST", xff: 0.5, steps: 1, rows: 10});
    assert(rra);
    
  });

  it('should output RRA spec', function () {	
    
    var rra = new RRA({cf:"LAST", xff: 0.5, steps: 1, rows: 10});
    assert(rra.toString);
    assert(rra.toString() === "RRA:LAST:0.5:1:10");
    
  });  
  
});


describe('rrd create', function() {
  
  rrd = require('../lib/rrd');
  var path = "/tmp/mocha-test-2.rrd"
  var DS = rrd.DS, RRA = rrd.RRA;
  var Strict = require('arguee').Strict, isa = require('arguee').isa;
  
  
  it('should create with DS and RRA objects', function (done) {
    
    rrd.create(path, {
      steps: 60,
      time: rrd.nows(-10),
      
      ds: new DS({name: "dc", type: "gauge", heartbeat: 120, min: 0, max: "U"}),
      rra: new RRA({cf: "last", xff: 0.5, steps: 1, rows: 10})
      
    }, function (error) { assert(!error); done(); });
    
  });
  
  it('should create with Array of DS and RRA objects', function (done) {
    
    rrd.create(path, {
      steps: 60,
      time: rrd.nows(-10),
      
      ds: [new DS({name: "dc", type: "gauge", heartbeat: 120, min: 0, max: "U"}), new DS({name: "dc2", type: "gauge", heartbeat: 120, min: 0, max: "U"})],
      rra: [new RRA({cf: "last", xff: 0.5, steps: 1, rows: 10}), new RRA({cf: "average", xff: 0.5, steps: 10, rows: 10})]
      
    }, function (error) { assert(!error); done(); });
    
  });

  it('should create with Objects for DS and RRA', function (done) {
      
    rrd.create(path, {
      steps: 60,
      time: rrd.nows(-10),
      
      ds: {name: "dc", type: "gauge", heartbeat: 120, min: 0, max: "U"},
      rra: {cf: "last", xff: 0.5, steps: 1, rows: 10}
      
    }, function (error) { assert(!error); done(); });    
    
  });
  
  it('should create with Array of Objects for DS and RRA', function (done) {
    
    rrd.create(path, {
      steps: 60,
      time: rrd.nows(-10),
      
      ds: [{name: "dc", type: "gauge", heartbeat: 120, min: 0, max: "U"}, {name: "dc2", type: "gauge", heartbeat: 120, min: 0, max: "U"}],
      rra: [{cf: "last", xff: 0.5, steps: 1, rows: 10}, {cf: "average", xff: 0.5, steps: 10, rows: 10}]
      
    }, function (error) { assert(!error); done(); });
  
  });
  
});


describe('rrd update', function() {
  
  rrd = require('../lib/rrd');
  var path = "/tmp/mocha-test-3.rrd"
  
  before(function (done) {
    rrd.create(path, {
      step: 1,
      time: rrd.nows(-1),
      
      ds: [{name: "dc1", type: "gauge", heartbeat: 5, min: 0, max: "U"}, {name: "dc2", type: "gauge", heartbeat: 5, min: 0, max: "U"}],
      rra: [{cf: "last", xff: 0.5, steps: 1, rows: 10}, {cf: "average", xff: 0.5, steps: 10, rows: 10}]
      
    }, function (error) { assert(!error); done(); });
  });
  
  after(function () {
    fs.unlinkSync(path);
  });
  
  it('should update with timed values', function (done) {
  
    rrd.update(path, [rrd.nows(), {dc1: 1, dc2: 2}], function (error) { assert(!error); done(); });
    
  });

  it('should update with timed values and fetch the same values', function (finished) {
    
    var t0 = rrd.nows()
    var updates = [
      [t0 +1, {dc1: 1, dc2: 6}],
      [t0 +2, {dc1: 2, dc2: 7}],
      [t0 +3, {dc1: 3, dc2: 8}],
      [t0 +4, {dc1: 4, dc2: 9}],
      [t0 +5, {dc1: 5, dc2: 10}]
    ];
    var index = 0;
    
    
    var enqueue = whenDone(function() {
      
      // console.log("\n** Updates:\n", updates);
      
      var fetched = [];
      rrd.fetch(path, { cf: "LAST", start: t0, end: t0 +5, resolution: 1 }, function (time, data) {
        
        if (time && data) {
          fetched.push([time, data]);
          return;
        }
        
        // Now let's control that the output is what we updated
        if (!time && !data) {
          // console.log("\n** fetched:\n", fetched);
          
          fetched.forEach(function (fetch, index) {
            if (index >= updates.length) return;
            
            // Times are equal ?
            assert(fetch[0] === updates[index][0], "fetched times differ from updated times");
            
            // data is equal ?
            assert(JSON.stringify(fetch[1]) === JSON.stringify(updates[index][1]), "fetched data differ from updated data");
          });
          
          finished();
          return;
        }
      });
      
    }, updates.length);
    
    var interval = setInterval(function () {
      
      if (index >= updates.length) {
        clearInterval(interval);
        return;
      }

      enqueue(function(done) {
        
        rrd.update(path, updates[index++], function (error) { 
          assert(!error, error);  
          done(); 
        });
        
      });
      
    }, 1000);
        
  });  
    
});
