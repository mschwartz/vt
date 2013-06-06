/*global require, exports, log */
(function() {
    "use strict";
 
    var console = require('console'),
        Thread = require('threads').Thread,
        InputStream = require('socket').InputStream,
        OutputStream = require('socket').OutputStream,
        Request = require('Request').Request,
        Response = require('Response').Response;

    function Child(serverSocket, fn) {
        this.on('exit', function() {
            log('exit');
            new Thread(Child, serverSocket, fn).start();
        });
        while (true) {
            serverSocket.mutex.lock();
            var sock = serverSocket.accept();
            serverSocket.mutex.unlock();

            var is = new InputStream(sock.fd),
                os = new OutputStream(sock.fd);

            var keepAlive = true;
            while (keepAlive) {
                try {
// var start = timer();
                    var request = new Request(is);
// log('> ' + sock.fd);
// log(timer() - start);
                    var response = new Response(os, request.proto);
// log(timer() - start);

                    request.threadId = this.threadId;

                    var connection = request.headers['connection'] || '',
                        headers = response.headers;
                    if (connection.toLowerCase() === 'keep-alive') {
                        headers['Connection'] = 'Keep-Alive';
                        headers['keep-alive'] = 'timeout: 5; max = 10000000';
                    }
                    else {
                        headers['Connection'] = 'close';
                        keepAlive = false;
                    }
// log(timer() - start);
                    fn.call(this, request, response);
// log(timer() - start);
// log(request.uri);
                }
                catch (e) {
                    if (e === 'EOF') {
                        // log('eof')
                        break;
                    }
                    log(e.toString());
                    console.dir(e);
                }
            }
            is.destroy();
            os.destroy();
            sock.destroy();
        }
    }

    exports.extend({
        Child: Child
    });

}());
