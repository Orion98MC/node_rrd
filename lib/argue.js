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

/*
	"argue" lets you check if arguments of a function conform to a given set of types

	Usage:

	conform = require('argue').conform;

	// Let's check how the user called the foo() function:
	function foo() {
		if (conform.apply([String, String, Number, Function], arguments)) {
			// Do this...
		} else if (conform.apply([String, Number], arguments)) {
			// Do that...
		} else {
			// Sorry we don't handle this kind of arguments
		}
	}

*/

var _types = {
	  String : function (v) { return (typeof v === 'string'); }
	, Array : function (v) { return (typeof v === 'object') && (v.constructor === Array); }
	, Object : function (v) { return (typeof v === 'object') && (v.constructor === Object); }
	, Number : function (v) { return (typeof v === 'number'); }
	, Function : function (v) { return (typeof v === 'function'); }
};

function _conform(fmt, arg) {
	if (typeof fmt !== 'string') { fmt = fmt.name; } 
	return _types[fmt](arg);
}

function conform() {
	if (false === this instanceof Array) { throw "(conform) *this* must be an Array"; }
	if (arguments.length < this.length) { return false; }

	var formatted = true
	, args = arguments;
	this.forEach(function (fmt, index) {
		if (!_conform(fmt, args[index])) {
			formatted = false;
			return;
		}
	});
	return formatted;
}


module.exports.conform = conform;
module.exports._types = _types;