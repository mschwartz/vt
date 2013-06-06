/*global require, exports */

(function() {
    "use strict";

    var Child = require('Child').Child,
        Thread = require('threads').Thread,
        Mutex = require('threads').Mutex,
        Socket = require('socket').Socket;

    function Server(fn) {
        this.fn = fn;
    }
    Server.prototype.extend({
        listen: function(port, bindAddress, numChildren) {
            numChildren = numChildren || 50;
            var serverSocket = new Socket();
            serverSocket.listen(port, bindAddress, 100);
            serverSocket.mutex = new Mutex();
            for (var i=0; i<numChildren; i++) {
                new Thread(Child, serverSocket, this.fn).start();
            }
        }
    });

    exports.extend({
        Server: Server
    });

}());
