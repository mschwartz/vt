/*global require, exports, log */
(function() {
    "use strict";
 
    var console = require('console'),
        Thread = require('threads').Thread,
        InputStream = require('socket').InputStream,
        OutputStream = require('socket').OutputStream,
        Request = require('Request').Request,
        Response = require('Response').Response;

var sss = [
    'HTTP/1.1 200 OK',
    'Content-type: text/plain',
    'Content-length: 3'
].join('\n') + '\n\nabc';

    function Child(serverSocket, fn) {
        this.on('exit', function() {
            log('exit');
            new Thread(Child, serverSocket, fn).start();
        });
        while (true) {
            serverSocket.mutex.lock();
            var sock = serverSocket.accept();
            serverSocket.mutex.unlock();

            // sock.write(sss, sss.length);
            // sock.destroy();
            // continue;

            var is = new InputStream(sock.fd),
                os = new OutputStream(sock.fd);

            var keepAlive = true;
            while (keepAlive) {
                try {
                    var request = new Request(is),
                        response = new Response(os, request.proto);

                    request.threadId = this.threadId;

                    if (request.headers['connection']) {
                        response.headers['Connection'] = 'Keep-Alive';
                        response.headers['keep-alive'] = 'timeout: 5; max = 10000000';
                    }
                    else {
                        response.headers['Connection'] = 'close';
                        keepAlive = false;
                    }
                    fn(request, response);
                }
                catch (e) {
                    if (e === 'EOF') {
                        break;
                    }
                    console.dir(e);
                }
            }
            log('is.destroy');
            is.destroy();
            log('os.destroy');
            os.destroy();
            log('sock.destroy');
            sock.destroy();
            log('to accept');
        }
    }

    exports.extend({
        Child: Child
    });

}());
