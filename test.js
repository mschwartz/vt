var console = require('console'),
    Thread = require('threads').Thread;

log('here');

var counter = 0;

function thread1() {
    while (1) {
        console.log('thread1');
    }
}

function thread2() {
    while (1) {
        console.log('thread2');
    }
}

function x(n) {
    while (1) {
        console.log('thread ' + n + ' ' + counter);
    }
}

function main() {
    log('main');
    // new Thread(thread1).start();
    // new Thread(thread2).start();
    for (var i=1; i<6; i++) {
        new Thread(x, i).start();
    }
    while (1) {
    	console.log('main');
        counter++;
    }

}
