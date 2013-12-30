var GLFW = module.exports = require('./build/Release/glfw.node');

// make sure cursor is always visible
//GLFW.Enable(GLFW.MOUSE_CURSOR);

// Easy event emitter based event loop.  Started automatically when the first
// listener is added.
var events;
Object.defineProperty(GLFW, 'events', {
  get: function () {
    if (events) return events;
    events = new (require('events').EventEmitter);
    
    var _emit=events.emit;
    events.emit=function() {
      var args = Array.prototype.slice.call(arguments);
      var evt= args[1]; // args[1] is the event, args[0] is the type of event
      //console.log("emitting event: "+require('util').inspect(args));
      if(args[0] != 'quit') {
        evt.preventDefault = function () {};
        evt.stopPropagation = function () {};
      }
      _emit.apply(this,args);
    };
    return events;
  },
  enumerable: true,
  configurable: true
});

