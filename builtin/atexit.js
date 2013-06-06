(function() {
	var exit_funcs = [];

	global.atexit = function(fn) {
		exit_funcs.push(fn);
	};

	global._atexit_ = function() {
		for (var i=0, len=exit_funcs.length; i<len; i++) {
			exit_funcs[i]();
		}
	};
	
}());