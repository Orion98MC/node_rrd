# Description

rrd is a node.js (http://nodejs.org) native binding for RRDtool (http://oss.oetiker.ch/rrdtool/).

# Dependencies

In order to compile the c++ sources, you need to have: 
  * node-gyp (npm install -g node-gyp)
  * a compiler toolchain (gcc etc...)
  * the RRDtool library (http://oss.oetiker.ch/rrdtool/).

For tests:
  * mocha (http://visionmedia.github.com/mocha/).

# Install

## Using git

	$ git clone <this repository>
	$ cd node_rrd
	$ npm install .

# Basic usage

Require the module

```js
var rrd = require('rrd');
```

In the following examples, we assume:

```js
var filename = '/tmp/test.rrd';
```

## create

```js
var now = Math.ceil((new Date).getTime() / 1000);

rrd.create(filename, 60, now, ["DS:busy:GAUGE:120:0:U", "RRA:LAST:0.5:1:60"], function (status){ 
	console.log(status === 0 ? "Created!" : "Error!");
});
```

## info

```js
var filename = '/tmp/test.rrd';

rrd.info(filename, function (info) {
	console.log(info.filename, info.rrd_version);
}); 
```

## last

```js
rrd.last(filename, function (last) {
	console.log(last);
});
```

## update

```js
var value = 80.0;
var now = Math.ceil((new Date).getTime() / 1000);

rrd.update('/tmp/test.rrd', 'busy', [[now, value].join(':')], function (status){ 
	console.log(status === 0 ? "Updated!" : "Error!");
});
```

## fetch

```js
var now = Math.ceil((new Date).getTime() / 1000);

rrd.fetch(filename, "LAST", now - 1000, now, null, function (time, data) { 
	console.log(time, data); 
});
```

# Example

Let's create an RRD to store the system load average over 1 minute for the last hour.

```js
#!/usr/bin/env node

var rrd = require('./build/Release/rrd')
,	os = require('os')
,	filename = '/tmp/test.rrd'
;

function now() { return Math.ceil((new Date).getTime() / 1000); }

function startUpdating() {
	function updateNow() {
		rrd.update(filename, "loadavg1", [[now(), os.loadavg()[0]].join(':')], function (status) {
			console.log(status === 0 ? '.' : '*');
		});
	}

	setInterval(updateNow, 60 * 1000);
}

rrd.create(filename, 60, now(), ['DS:loadavg1:GAUGE:120:0:U', 'RRA:LAST:0.5:1:60'], function (status) {
	if (status !== 0) throw 'Error creating RRD';

	console.log("RRD created!");
	startUpdating();
});
```

# Alpha

This module is in alpha development stage. 
At this point not all rrdtool commands are binded. I did the ones that where the most important to me.
If you really need xport or an other command feel free to contact me (thierry.passeron@gmail.com).

# Tests

There are some (few) tests in /test done with mocha (http://visionmedia.github.com/mocha/).
I plan to include more tests in the future.

# License terms

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
