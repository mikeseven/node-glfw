var GLFW = module.exports = require('./build/Release/node_glfw.node');

// make sure cursor is always visible
GLFW.Enable(GLFW.MOUSE_CURSOR);

// Easy event emitter based event loop.  Started automatically when the first
// listener is added.
var events;
Object.defineProperty(GLFW, 'events', {
  get: function () {
    if (events) return events;
    events = new (require('events').EventEmitter);
    var now = Date.now();
    setInterval(function () {
      var after = Date.now();
      var delta = after - now;
      now = after;
      var data;
      /*while (data = GLFW.pollEvent()) {
        events.emit('event', data);
        events.emit(data.type, data);
      }*/
      events.emit('tick', delta);
    }, 16);
    return events;
  }
});

