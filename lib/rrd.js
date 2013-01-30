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

var bindings = require('bindings')('rrd_bindings')
,   conformed = require('arguee').conformed
,   isa = require('arguee').isa
,   Strict = require('arguee').Strict
,   fs = require('fs')
;


/*

  Tools

*/

/* @return seconds from epoch for a Date argument or a miliseconds Number argument 
  if resolution is set return the nearest seconds within this resolution
*/
var toSeconds = function (d, resolution) {
  resolution = resolution || 1;
  if (conformed([Date], [d])) { /* date to seconds since epoch */
    return Math.floor(d.getTime() / (1000 * resolution)) * resolution;
  }
  if (conformed([Number], [d])) { /* miliseconds since epoch to seconds since epoch */
    return Math.floor(d / (1000 * resolution)) * resolution;
  }
  throw new Error("toSeconds: format not handled");
}

/* @returns new Date with seconds from now */
var now = function (seconds) {
  seconds = seconds || 0;
  var _now = (new Date).getTime();
  return new Date(_now + seconds * 1000);
}

/* @returns seconds since epoch with seconds from now */
var nows = function (seconds) {
  return toSeconds(now(seconds));
}



/*

  DS and RRA helper methods

*/

function DS(ds) {
  var self = this;
  ["type", "name"].forEach(function(k) {
    if (!isa(String, ds[k])) throw new Error('rrd: DS: ' + k + ' is required and must be a String (' + JSON.stringify(ds) + ')');
    self[k] = ds[k];
  });
  
  this.type = this.type.toUpperCase();
  
  if (this.type === "COMPUTE") {
    if (!ds.expression || !isa(String, ds.expression)) {
      throw new Error('rrd: DS: COMPUTE type requires an expression key (' + JSON.stringify(ds) + ')');
    }
    this.expression = ds.expression;
  } else {
    ["heartbeat", "min", "max"].forEach(function (k) {
      if (isa(undefined, ds[k])) throw new Error('rrd: DS: ' + k + ' is required (' + JSON.stringify(ds) + ')');
      self[k] = "" + ds[k];
    });
  } 
}

DS.prototype.toString = function () {
  return (this.type === "COMPUTE" ? ["DS", this.name, this.type, this.expression] : ["DS", this.name, this.type, this.heartbeat, this.min, this.max]).join(':');
};

DS.prototype.constructor = DS;


/* 
  RRA Creation facility 

  At the moment, it only supports standard CF:
  
  RRA:AVERAGE | MIN | MAX | LAST:xff:steps:rows
  
  for other CF you must use the create(String path, Number steps, Number time, Array specs, Function callback):
  
*/
function RRA(rra) {
  var self = this;
  ["cf", "xff", "steps", "rows"].forEach(function(k) {
    if (isa(undefined, rra[k])) throw new Error('rrd: RRA:' + k + ' is required (' + JSON.stringify(rra) + ')');
    self[k] = rra[k];
  });
  
  this.cf = this.cf.toUpperCase();
}

RRA.prototype.toString = function () {
  return ["RRA", this.cf, this.xff, this.steps, this.rows].join(':');
};

RRA.prototype.constructor = RRA;




/*

  RRDtool

*/


/*

  create(String path, Object options, Function callback(Error error))
  
    options:
    {
      step: <seconds>,
      time: <seconds-since-epoch>,
    
      ds: DSs
      rra: RRAs
    }
  
    DSs:
      using rrd.DS (allows you to check the validity of the DS at creation)
      [ new rrd.DS({name:<name>, type:<type>, ...}), ... ]
      
      or directly
      [{name:<name>, type:<type>, ...}, ...]
      
      or with only one DS
      {name:<name>, type:<type>, ...}
  
    RRAs:
      using rrd.RRA (allows you to check the validity of the RRA at creation):
      [new rrd.RRA({cf:<type>, ...}), ...]
      
      or directly
      [{cf:<type>, ...}, ...]
      
      or with only one RRA
      {cf:<type>, ...}
      
    example:
    --------
      create('./devices.rrd', {
      
        step: 3600,
        time: rrd.nows(-10),
        
        ds:  new DS({ name: 'dc', type: 'GAUGE', heartbeat: 300, min: 0, max: "U" }),
        rra: new RRA({ cf: 'LAST', xff: 0.5, steps: 1, rows: 72 }),
        
      }, function (error) {
        if (error) console.log("oups!", error);
      })
    
  OR
  
  create(String path, Number steps, Number time, Array specs, Function callback)
  
    specs:
      ["DS:...", "DS:...", ..., "RRA:...", "RRA:...", ...]

*/
var create = function () {
  var args = null;
  
  if (args = conformed([String, Strict(Object), Function], arguments)) {
    
    var options = {
      step: 60,         /* how many seconds separate 2 updates */
      time: nows(-10),   /* seconds since epoch */
      
      ds: [],
      rra: []
    };
    
    // Merge options
    for (var k in args[1]) { if (options.hasOwnProperty(k)) options[k] = args[1][k]; }
    
    // Date
    if (isa(Date, options.time)) options.time = toSeconds(options.time);
    
    // Build DS and RRA
    if (isa(Strict(Object), options.ds)) options.ds = [new DS(options.ds)];
    if (isa(DS, options.ds)) options.ds = [options.ds];
    if (isa(Strict(Object), options.rra)) options.rra = [new RRA(options.rra)];
    if (isa(RRA, options.rra)) options.rra = [options.rra];
    
    var specs = [];
    options.ds.forEach(function (ds) { 
      if (isa(Strict(Object), ds)) specs.push((new DS(ds)).toString());
      else if (isa(DS, ds)) specs.push(ds.toString()); 
      else throw new Error("rrd: create: ds key requires rrd.DS objects");
    });
    options.rra.forEach(function (rra) { 
      if (isa(Strict(Object), rra)) specs.push((new RRA(rra)).toString());
      else if (isa(RRA, rra)) specs.push(rra.toString()); 
      else throw new Error("rrd: create: rra key requires rrd.RRA objects");
    });
    
    // console.log("rrdtool create", args[0], options['step'], options['time'], specs);
    return bindings.create.call(null, args[0], options['step'], options['time'], specs, args[2]);
    
  }
  
  if (conformed([String, Number, Number, Array, Function], arguments)) {
    return bindings.create.apply(null, arguments);
  }
  
  throw new Error("rrd: arguments' format not implemented!");
};

/*

update(String path, Array timed-values, Function callback (Error error));

  timed-values:
  
  [time1, {
    "ds-name1": value1,
    "ds-name2": value2,
    ...
  }]
  
  example:  
  --------
    update("./myrrd.rrd", [time, { uc: 1234, dc: 2345 }], function (error) { 
      if (error) console.log("oups!", error);
    });


OR

update(String path, String template, Array updates, Function callback);

  example:  
  --------
    update("./myrrd.rrd", "uc:dc", ["1234567890:1234:2345"], function (error) { 
      if (error) console.log("oups!", error);
    });


*/
var update = function () {
  var args = null;
  
  if (args = conformed([String, Array, Function], arguments)) {
    var templates = [], values = args[1][1], updates = [];
    
    updates.push(args[1][0] /* time */)
    for (var ds_name in values) {
      templates.push(ds_name);
      updates.push(values[ds_name]); 
    }
    // console.log('Should update with:', args[0], templates.join(':'), [updates.join(':')]);
    return bindings.update.call(null, args[0], templates.join(':'), [updates.join(':')], args[2]);
  }
  
  if (conformed([String, String, Array, Function], arguments)) {
    return bindings.update.apply(null, arguments);
  } 
  
  throw new Error("rrd: arguments' format not implemented!");
};


/* 

  Fetch from a rrd file 

  fetch(String path, Object options, Function callback)
  
    options:
    {
      cf: <cf-string>,           
      start: <seconds-since-epoch>,
      end: <seconds-since-epoch>,
      resolution: <seconds>
    }
    
    example:  
    --------
      fetch("./myrrd.rrd", {
        
        cf: "LAST",
        start: rrd.nows(-120),
        end: rrd.nows(-10),
        resolution: 10
        
      }, function (error) { 
          if (error) console.log("oups!", error);
      });
  
  
*/
var fetch = function () {
  var args = null;
  
  if (args = conformed([String, Strict(Object), Function], arguments)) {
    
    var options = {
      cf: "LAST",           
      start: nows(-3600),   /* seconds since epoch */
      end: nows(),          /* seconds since epoch */
      resolution: 60        /* seconds */
    };
    
    // Merge options
    for (var k in args[1]) { if (options.hasOwnProperty(k)) options[k] = args[1][k]; }
    
    // Check options for dates
    ['start', 'end'].forEach(function(k) {
      if (isa(Date, options[k])) { options[k] = toSeconds(options[k]); }
    });
    
    return bindings.fetch.call(null, args[0], options['cf'], options['start'], options['end'], options['resolution'], args[2]);
    
  }
  
  throw new Error("rrd: arguments' format not implemented!");
};


var last = function () {
  if (conformed([String], arguments)) return bindings.last.apply(null, arguments);
  
  throw new Error("rrd: arguments' format not implemented!");
};

var info = function () {
  if (conformed([String], arguments)) return bindings.info.apply(null, arguments);
  
  throw new Error("rrd: arguments' format not implemented!");
};



var RRD = {
      core: bindings
      
    , create: create
    , update: update
    , fetch: fetch
    , last: last
    , info: info
    
    , toSeconds: toSeconds
    , nows: nows
    , now: now
    
    , DS: DS
    , RRA: RRA
}


module.exports = RRD;
