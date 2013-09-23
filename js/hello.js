var jbProgram = '' +
'; "Hello, World!" for MicroIO\n' +
'\n' +
' .device "microio"\n' +
'\n' +
' ldx #0\n' +
'loop:\n' +
' lda msg,x\n' +
' beq quit\n' +
' sta 2:52,x\n' +
' inx\n' +
' bne loop\n' +
'\n' +
'quit:\n' +
' brk\n' +
'\n' +
'.data\n' +
'msg:\n' +
' "Hello,    World!" 0\n' +
'';
