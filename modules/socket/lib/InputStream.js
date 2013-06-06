/*global require, exports */
(function() {
    "use strict";

    var Memory = require('binary').Memory,
        async = require('builtin.async'),
        console = require('console'),
        usleep = require('builtin.process').usleep;

    function InputStream(fd) {
        this.fd = fd;
        this.eof = false;
        this.buffer = new Memory(4096);
        this.offset = 0;
        this.end = 0;
    }
    
    InputStream.prototype.extend({
        destroy: function() {
            this.buffer.destroy();
        },
        read: function() {
            if (this.eof) {
                throw 'EOF';
            }
            var buffer = this.buffer,
                fd = this.fd;

            if (this.offset >= this.end) {
                this.offset = 0;
                this.end = 0;
                while (this.end === 0) {
                    if (!async.readable(fd, 1, 0)) {
                        this.eof = true;
                        throw 'EOF';
                    }
                    this.end = buffer.read(fd, 0, 4096);
                    if (this.end <= 0 || this.end === null) {
                        this.eof = true;
                        throw 'EOF';
                    }
                }
            }
            return buffer.getAt(this.offset++);
        },

        readLine: function() {
            var fd = this.fd,
                buffer = this.buffer,
                offset = this.offset,
                end = this.end,
                line = '',
                nl;
            while (true) {
                while (offset >= end) {
                    if (!async.readable(fd, 1, 0)) {
                        throw 'EOF';
                    }
                    end = buffer.read(fd, 0, 4096);
                    if (end <= 0 || end === null) {
                        throw 'EOF';
                    }
                    offset = 0;
                }
                line += buffer.asString(offset, end-offset);
                nl = line.indexOf('\n');
                if (nl !== -1) {
                    break;
                }
                offset = end = 0;
            }
            this.offset = offset + nl + 1;
            this.end = end;
            return line.substr(0, nl-1);
        },

        xreadLine: function() {
            // console.dir('readLine');
            var line = '',
                inp;
            while ((inp = this.read())) {
                // console.dir('line = "' + line + '"');
                switch (inp) {
                    case '\r':
                        continue;
                    case '\n':
                        // log(line);
                        return line;
                    default:
                        line += inp;
                }
            }
            this.eof = true;
            throw 'EOF';
        }
    });

    exports.extend({
        InputStream: InputStream
    });

}());