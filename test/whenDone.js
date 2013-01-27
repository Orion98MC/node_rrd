/*
  whenDone

  Run a specific function when done.

  Usage:

    var enqueue = whenDone(function () { console.log('All done'); });

    enqueue(function(done) { async_work1(done); } );
    enqueue(function(done) { async_work2(done); } );
    ...

    OR
    enqueue(function(done) { 
      async_work1(done); 
      async_work2(done);
    }, 2 ); // <--- Notice the *2* meaning you wait for 2 *done* 


  Copyright (c) 2012, Thierry Passeron

  MIT License
*/

function whenDone(doneCallback, left) {
  var left = left || 0
  , done = function() { if (--left === 0) { doneCallback(); } };
  return function doThis(work, n) {
    if (typeof n !== 'undefined') left += n;
    // else left++;
    process.nextTick(function(){ work(done); });
  };
}

module.exports = whenDone;