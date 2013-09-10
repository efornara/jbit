n_of_gets = 0;

function addr_to_str(addr) {
	return (addr >> 8) + ":" + (addr & 0xff);
}

JBIT = {
	io_put: function(addr,value) {
		console.log('io_put(' + addr_to_str(addr) + ', ' + value + ')');
	},
	io_get: function(addr) {
		var res = 0;
		if (n_of_gets++ > 5)
			res = 42;
		console.log('io_get(' + addr_to_str(addr) + ') => ' + res);
		return res;
	}
};

Module.ccall('vm_load', undefined, ['string'], ['Hello, World!']);
vm_step = Module.cwrap('vm_step', 'number', []);
while (vm_step() == 1)
	;
