var http = require('http'),
    console = require('console'),
    process = require('builtin.process');

console.dir(builtin.async);

http.createServer(function(req, res) {
    // console.log(req.uri);
    res.writeHead(200, { 'Content-Type': 'text/plain'});
    res.end('Hello World\n');
}).listen(1337, '127.0.0.1');
console.log('Server running at http://127.0.0.1:1337/');
while(1) { process.sleep(111); }
