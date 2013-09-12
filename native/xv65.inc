;
; xv65.inc
;

;
; Copyright (C) 2012-2013  Emanuele Fornara
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions
; are met:
;  * Redistributions of source code must retain the above copyright
;    notice, this list of conditions and the following disclaimer.
;  * Redistributions in binary form must reproduce the above copyright
;    notice, this list of conditions and the following disclaimer in
;    the documentation and/or other materials provided with the
;    distribution.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
; LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
; FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
; COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
; INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
; BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
; OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
; AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
; OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
; OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
; SUCH DAMAGE.
;

PUTCHAR = $0200

REQPUT = $0201
REQEND = $0202
REQRES = $0203

REQPTRLO = $0204
REQPTRHI = $0205

;DEBUGLEV = $0210
REQERRNO = $0211
PIDSIZE = $0212

REQDAT = $0260

REQ_DEBUG = 0

REQ_SYS_fork   =  1
REQ_SYS_exit   =  2
REQ_SYS_wait   =  3
REQ_SYS_pipe   =  4
REQ_SYS_read   =  5
REQ_SYS_kill   =  6
REQ_SYS_exec   =  7
REQ_SYS_fstat  =  8
REQ_SYS_chdir  =  9
REQ_SYS_dup    = 10
REQ_SYS_getpid = 11
REQ_SYS_sbrk   = 12
REQ_SYS_sleep  = 13
REQ_SYS_uptime = 14
REQ_SYS_open   = 15
REQ_SYS_write  = 16
REQ_SYS_mknod  = 17
REQ_SYS_unlink = 18
REQ_SYS_link   = 19
REQ_SYS_mkdir  = 20
REQ_SYS_close  = 21

; STANDARD SYMBOLS

SIGHUP   = 1
SIGINT   = 2
SIGQUIT  = 3
SIGILL   = 4
SIGABRT  = 6
SIGFPE   = 8
SIGKILL  = 9
SIGSEGV = 11
SIGPIPE = 13
SIGALRM = 14
SIGTERM = 15

EPERM   =  1
ENOENT  =  2
ESRCH   =  3
EINTR   =  4
EIO     =  5
ENXIO   =  6
E2BIG   =  7
ENOEXEC =  8
EBADF   =  9
ECHILD  = 10
EAGAIN  = 11
ENOMEM  = 12
EACCES  = 13
EFAULT  = 14
ENOTBLK = 15
EBUSY   = 16
EEXIST  = 17
EXDEV   = 18
ENODEV  = 19
ENOTDIR = 20
EISDIR  = 21
EINVAL  = 22
ENFILE  = 23
EMFILE  = 24
ENOTTY  = 25
ETXTBSY = 26
EFBIG   = 27
ENOSPC  = 28
ESPIPE  = 29
EROFS   = 30
EMLINK  = 31
EPIPE   = 32
EDOM    = 33
ERANGE  = 34

; ASM ONLY

IO = $0200
IO_HI = $02

.macro  req
@_req_: .word @_endreq_ - @_req_ - 2
.endmacro

.macro  endreq
@_endreq_:
.endmacro
