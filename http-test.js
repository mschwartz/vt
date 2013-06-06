var http = require('http'),
    console = require('console'),
    process = require('builtin.process');

var counter = 0;

http.createServer(function(req, res) {
    // console.log(req.uri);
    var tid = this.threadId;
    res.writeHead(200, { 'Content-Type': 'text/plain'});
    // res.end('Hello World\n');
    res.end(tid + ' Hello World ' + counter + '\n');
    counter++;
}).listen(1337, '127.0.0.1');
console.log('Server running at http://127.0.0.1:1337/');
//builtin.pthread._exit();
//while (1) { process.sleep(1000); }

