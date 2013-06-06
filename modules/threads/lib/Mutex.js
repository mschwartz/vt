(function() {
    var pthread = require('builtin/pthread'),
        console = require('console'),
        Thread = require('Thread').Thread;

    function debug(s) {
        console.dir(pthread.t() + ' ' + Thread.currentThread().threadId + ' ' + s);
    }

    var mutexes = {},
        mutexId = 0;

    function Mutex() {
        this._mutex = pthread.mutex_init();
        this.id = ++mutexId;
        this.owner = null;
        mutexes[this.id] = this;
    }
    Mutex.unlock = function(t) {
        if (t.lockCount) {
            var threadId = t.threadId;
            mutexes.each(function(mutex) {
                if (mutex.owner.threadId === threadId) {
                    log('unlocking!');
                    pthread.mutex_unlock(mutex._mutex);
                    t.lockCount--;
                    log('unlocked!');
                }
            });
            log('done');
        }
    };
    Mutex.prototype.extend({
        destroy: function() {
            return pthread_destroy_mutex(this._mutex);
        },
        lock: function() {
            var ret = pthread.mutex_lock(this._mutex);
            var t = Thread.currentThread();
            this.owner = t;
            if (t) {
                t.lockCount++;
            }
            return ret;
        },
        trylock: function() {
            var ret =  pthread.mutex_try_lock(this._mutex);
            var t = Thread.currentThread();
            this.owner = t;
            if (t) {
                t.lockCount++;
            }
            return ret;
        },
        unlock: function() {
            var ret = pthread.mutex_unlock(this._mutex);
            var t = Thread.currentThread();
            this.owner = null;
            if (t) {
                t.lockCount--;
            }
            return ret;
        }
    });
    exports.Mutex = Mutex;
}());
