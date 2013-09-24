var jbProgram = '' +
'; "Hello, World!" for MicroIO\n' +
'\n' +
'\t.device "microio"\n' +
'\n' +
'\tldx #0\n' +
'loop:\n' +
'\tlda msg,x\n' +
'\tbeq quit\n' +
'\tsta 2:52,x\n' +
'\tinx\n' +
'\tbne loop\n' +
'\n' +
'quit:\n' +
'\tbrk\n' +
'\n' +
'.data\n' +
'msg:\n' +
'\t"Hello,    World!" 0\n' +
'';
