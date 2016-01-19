/*
 * Copyright (C) 2012-2013  Emanuele Fornara
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

// xv65.cpp

#define _FILE_OFFSET_BITS 64
#define _LARGE_FILES 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <ctype.h>

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "jbit.h"
#include "devimpl.h"

#include "_xv65.h"

#define IO_BASE 0x200
#define ERR XV65_EINVAL

namespace {

extern "C" void sig_handler(int) {
}

const char *req_id_to_string(int id) {
	switch (id) {
	case 1: return "REQ_FORK";
	case 2: return "REQ_EXIT";
	case 3: return "REQ_WAIT";
	case 4: return "REQ_PIPE";
	case 5: return "REQ_READ";
	case 6: return "REQ_KILL";
	case 7: return "REQ_EXEC";
	case 8: return "REQ_FSTAT";
	case 9: return "REQ_CHDIR";
	case 10: return "REQ_DUP";
	case 11: return "REQ_GETPID";
	case 12: return "REQ_SBRK";
	case 13: return "REQ_SLEEP";
	case 14: return "REQ_UPTIME";
	case 15: return "REQ_OPEN";
	case 16: return "REQ_WRITE";
	case 17: return "REQ_MKNOD";
	case 18: return "REQ_UNLINK";
	case 19: return "REQ_LINK";
	case 20: return "REQ_MKDIR";
	case 21: return "REQ_CLOSE";
	case 32: return "REQ_ARGC";
	case 33: return "REQ_ARGV";
	case 34: return "REQ_ENV";
	case 48: return "REQ_TIME";
	case 49: return "REQ_RMDIR";
	case 50: return "REQ_LSEEK";
	default: return "?";
	}
}

const char *errno_to_string(int id) {
	switch (id) {
	case 1: return "EPERM";
	case 2: return "ENOENT";
	case 3: return "ESRCH";
	case 4: return "EINTR";
	case 5: return "EIO";
	case 6: return "ENXIO";
	case 7: return "E2BIG";
	case 8: return "ENOEXEC";
	case 9: return "EBADF";
	case 10: return "ECHILD";
	case 11: return "EAGAIN";
	case 12: return "ENOMEM";
	case 13: return "EACCES";
	case 14: return "EFAULT";
	case 15: return "ENOTBLK";
	case 16: return "EBUSY";
	case 17: return "EEXIST";
	case 18: return "EXDEV";
	case 19: return "ENODEV";
	case 20: return "ENOTDIR";
	case 21: return "EISDIR";
	case 22: return "EINVAL";
	case 23: return "ENFILE";
	case 24: return "EMFILE";
	case 25: return "ENOTTY";
	case 26: return "ETXTBSY";
	case 27: return "EFBIG";
	case 28: return "ENOSPC";
	case 29: return "ESPIPE";
	case 30: return "EROFS";
	case 31: return "EMLINK";
	case 32: return "EPIPE";
	case 33: return "EDOM";
	case 34: return "ERANGE";
	default: return "?";
	}
}

#define STR_ESC_CLEAR "\x1b[2J"
#define STR_ESC_HOME "\x1b[;H"
#define STR_ESC_NORMAL "\x1b[0m"

class Xv65Device : public Device {
private:
	const static int top_memory = 0xffff;
	AddressSpace *m;
	int argc;
	char **argv;
	Buffer req_buf;
	Buffer tmp_buf;
	Random random;
	MicroIODisplay display;
	bool microio;
	bool microio_refresh;
	int v_REQPTRHI;
	int v_REQRES;
	int v_REQERRNO;
	int v_FRMFPS;
	int v_TRCLEVEL;
	int v_ERREXIT;
	FILE *putfp;
	unsigned char v_REQDAT[16];
	const char *r;
	int n;
	void put_value(unsigned char *buf, int size, uint64_t value) {
		int pattern = 0x01020304;
		int patter_v0 = *(unsigned char *)&pattern;
		unsigned char *v = (unsigned char *)&value;
		if (patter_v0 == 0x01) {
			for (int i = 0; i < size; i++)
				buf[i] = v[sizeof(uint64_t) - 1 - i];
		} else if (patter_v0 == 0x04) {
			for (int i = 0; i < size; i++)
				buf[i] = v[i];
		} else 
			abort();
	}
	void get_value(const unsigned char *buf, int size, uint64_t *value) {
		uint64_t v = 0;
		for (int i = 0; i < size; i++) {
			v <<= 8;
			v |= buf[size - i - 1];
		}
		*value = v;
	}
	int r_get_uint8(int i) {
		return r[i] & 0xff;
	}
	int r_get_uint16(int i) {
		return (r_get_uint8(i + 1) << 8) | r_get_uint8(i);
	}
	bool r_get_trailing_int64(int i, int64_t *value) {
		int len = n - i;
		if (len < 0 || len > 8)
			return false;
		int64_t v = 0;
		if (len > 0)
			v = r[i + len - 1] & 0x80 ? -1 : 0; 
		while (len > 0) {
			v <<= 8;
			v |= r[i + len - 1] & 0xff;
			len--;
		}
		*value = v;
		return true;
	}
	bool r_get_trailing_uint(int i, unsigned int *value) {
		int len = n - i;
		if (len < 0 || len > 8)
			return false;
		uint64_t v;
		get_value((const unsigned char *)&r[i], len, &v);
		*value = (int)v;
		return true;
	}
	int r_parse_string(int i) {
		for (; i < n; i++)
			if (r[i] == 0)
				return i + 1; // pos after string
		return -1;
	}
	int m_get_uint8(int addr) {
		int res = 0;
		if (addr >= 0 && addr <= top_memory)
			res =  m->get(addr) & 0xff;
		return res;
	}
	int m_get_uint16(int addr) {
		return (m_get_uint8(addr + 1) << 8) | m_get_uint8(addr);
	}
	int req_FORK() {
		if (n != 1)
			return ERR;
		pid_t pid = fork();
		if (pid == -1)
			return errno;
		put_value(v_REQDAT, 8, pid);
		return 0;
	}
	int req_EXIT() {
		unsigned int status;
		if (!r_get_trailing_uint(1, &status))
			return ERR;
		exit((int)status);
		return 0; // not reached
	}
	int req_WAIT() {
		if (n != 1)
			return ERR;
		int status;
		pid_t pid = wait(&status);
		if (pid == -1)
			return errno;
		put_value(&v_REQDAT[0], 8, pid);
		put_value(&v_REQDAT[8], 8, status);
		return 0;
	}
	int req_PIPE() {
		if (n != 1)
			return ERR;
		int p[2];
		int ret = pipe(p);
		if (ret < 0)
			return errno;
		put_value(&v_REQDAT[0], 8, p[0]);
		put_value(&v_REQDAT[8], 8, p[1]);
		if (p[0] > 255 || p[1] > 255)
			return XV65_ERANGE;
		return 0;
	}
	int req_KILL() {
		int sig;
		switch (n) {
		case 1:
			sig = SIGTERM;
			break;
		case 2:
			sig = r_get_uint8(1);
			break;
		default:
			return ERR;
		}
		uint64_t pid;
		get_value(v_REQDAT, sizeof(pid_t), &pid);
		int ret = kill((pid_t)pid, sig);
		return ret == -1 ? errno : 0;
	}
	int req_GETPID() {
		if (n != 1)
			return ERR;
		pid_t pid = getpid();
		put_value(v_REQDAT, 8, pid);
		return 0;
	}
	int req_SLEEP() {
		unsigned int seconds;
		if (!r_get_trailing_uint(1, &seconds))
			return ERR;
		int ret;
		if (seconds == 0) {
			uint64_t usec;
			get_value(v_REQDAT, 4, &usec);
#if !defined(ANDROID) && !defined(__ANDROID__)
			ret =
#else
			ret = 0;
#endif
			usleep(usec);
		} else {
			ret = sleep(seconds);
		}
		if (ret) {
			put_value(v_REQDAT, 8, ret);
			return XV65_EINTR;
		}
		return 0;
	}
	int req_EXEC() {
		int i = r_parse_string(1);
		if (i == -1)
			return ERR;
		const char *filename = &r[1];
		Buffer arr_buf;
		int n_of_args = 0;
		while (i < n) {
			int arg_i = i;
			i = r_parse_string(arg_i);
			if (i == -1)
				return ERR;
			const char *arg = &r[arg_i];
			char **slot = (char **)arr_buf.append_raw(sizeof(char *));
			memcpy(slot, &arg, sizeof(char *));
			n_of_args++;
		}
		const char *null = 0;
		char **slot = (char **)arr_buf.append_raw(sizeof(char *));
		memcpy(slot, &null, sizeof(char *));
		execvp(filename, (char* const*)arr_buf.get_data());
		return errno;
	}
	int req_FSTAT() {
		if (n != 2)
			return ERR;
		int fd = r_get_uint8(1);
		struct stat sb;
		int ret = fstat(fd, &sb);
		if (ret != 0)
			return errno; 
		int type;
		switch (sb.st_mode & S_IFMT) {
		case S_IFBLK:  type = XV65_T_BLK;  break;
		case S_IFCHR:  type = XV65_T_CHR;  break;
		case S_IFDIR:  type = XV65_T_DIR;  break;
		case S_IFIFO:  type = XV65_T_FIFO; break;
		case S_IFLNK:  type = XV65_T_LNK;  break;
		case S_IFREG:  type = XV65_T_REG;  break;
		case S_IFSOCK: type = XV65_T_SOCK; break;
		default:       type = 0;      break;
		}
		v_REQDAT[0] = type;
		put_value(&v_REQDAT[8], 8, sb.st_size);
		return 0;
	}
	int req_OPEN() {
		int i = r_parse_string(1);
		if (i == -1)
			return ERR;
		const char *filename = &r[1];
		if (n - i != 1)
			return ERR;
		int in_flags = r_get_uint8(i);
		int out_flags = 0;
		if (in_flags & XV65_O_CREAT)
			out_flags |= O_CREAT;
		if (in_flags & XV65_O_TRUNC)
			out_flags |= O_TRUNC;
		if (in_flags & XV65_O_APPEND)
			out_flags |= O_APPEND;
		if (in_flags & XV65_O_EXCL)
			out_flags |= O_EXCL;
		in_flags &= 0x0f;
		switch (in_flags) {
		case XV65_O_RDONLY:
			out_flags |= O_RDONLY;
			break;
		case XV65_O_WRONLY:
			out_flags |= O_WRONLY;
			break;
		case XV65_O_RDWR:
			out_flags |= O_RDWR;
			break;
		default:
			return ERR;
		}
		int fd = open(filename, out_flags, 0666);
		if (fd == -1)
			return errno;
		put_value(v_REQDAT, 8, fd);
		if (fd > 255)
			return XV65_ERANGE;
		return 0;
	}
	int req_rdwr(int req_id) {
		if (n != 6)
			return ERR;
		int fd = r_get_uint8(1);
		int addr = r_get_uint16(2);
		int len = r_get_uint16(4);
		if (addr + len > top_memory)
			return ERR;
		tmp_buf.reset();
		char *raw = tmp_buf.append_raw(len);
		int ret;
		if (req_id == REQ_READ) {
			ret = read(fd, raw, len);
			if (ret > 0)
				for (int i = 0; i < ret; i++)
					m->put(addr + i, raw[i]);
		} else if (req_id == REQ_WRITE) {
			for (int i = 0; i < len; i++)
				raw[i] = m->get(addr + i);
			ret = write(fd, raw, len);
		} else {
			return ERR;
		}
		if (ret < 0)
			return errno;
		put_value(v_REQDAT, 8, ret);
		return 0;
	}
	int req_CLOSE() {
		if (n != 2)
			return ERR;
		int fd = r_get_uint8(1);
		int ret = close(fd);
		return ret == -1 ? errno : 0;
	}
	int req_DUP() {
		if (n != 2)
			return ERR;
		int fd = r_get_uint8(1);
		fd = dup(fd);
		if (fd == -1)
			return errno;
		put_value(v_REQDAT, 8, fd);
		if (fd > 255)
			return XV65_ERANGE;
		return 0;
	}
	int req_filename(int req_id) {
		int i = r_parse_string(1);
		if (i == -1)
			return ERR;
		const char *filename = &r[1];
		if (n - i != 0)
			return ERR;
		int ret;
		switch (req_id) {
		case REQ_CHDIR:
			ret = chdir(filename);
			break;
		case REQ_MKDIR:
			ret = mkdir(filename, 0777);
			break;
		case REQ_UNLINK:
			ret = unlink(filename);
			break;
		case REQ_RMDIR:
			ret = rmdir(filename);
			break;
		default:
			return ERR;
		}
		return ret == -1 ? errno : 0;
	}
	int req_ARGC() {
		if (n != 1)
			return ERR;
		put_value(v_REQDAT, 8, argc);
		return 0;
	}
	int put_string(int addr, int size, const char *s) {
		if (addr + size > top_memory)
			return ERR;
		int str_size = strlen(s) + 1;
		put_value(v_REQDAT, 8, str_size);
		if (size >= str_size) {
			int i;
			for (i = 0; s[i]; i++)
				m->put(addr + i, s[i]);
			m->put(addr + i, 0);
		}
		return 0;
	}
	int req_ARGV() {
		if (n < 5)
			return ERR;
		int addr = r_get_uint16(1);
		int size = r_get_uint16(3);
		unsigned int i;
		if (!r_get_trailing_uint(5, &i))
			return ERR;
		if (i >= (unsigned int)argc)
			return XV65_EDOM;
		return put_string(addr, size, argv[i]);
	}
	int req_ENV() {
		if (n <= 5)
			return ERR;
		int addr = r_get_uint16(1);
		int size = r_get_uint16(3);
		int i = r_parse_string(5);
		if (i == -1)
			return ERR;
		const char *name = &r[5];
		if (n - i != 0)
			return ERR;
		const char *value = getenv(name);
		if (!value) {
			put_value(v_REQDAT, 8, 0);
			return 0;
		}
		return put_string(addr, size, value);
	}
	int req_TIME() {
		if (n != 1)
			return ERR;
		struct timeval tv;
		gettimeofday(&tv, NULL);
		put_value(&v_REQDAT[0], 8, tv.tv_sec);
		put_value(&v_REQDAT[8], 8, tv.tv_usec);
		return 0;
	}
	int req_LSEEK() {
		if (n < 3)
			return ERR;
		int fd = r_get_uint8(1);
		int in_whence = r_get_uint8(2);
		int64_t offset;
		if (!r_get_trailing_int64(3, &offset))
			return ERR;
		int out_whence;
		switch (in_whence) {
		case XV65_SEEK_SET:
			out_whence = SEEK_SET;
			break;
		case XV65_SEEK_CUR:
			out_whence = SEEK_CUR;
			break;
		case XV65_SEEK_END:
			out_whence = SEEK_END;
			break;
		default:
			return ERR;
		}
		off_t ret = lseek(fd, (off_t)offset, out_whence);
		if (ret < 0)
			return errno;
		put_value(&v_REQDAT[0], 8, (uint64_t)ret);
		return 0;
	}
	void dump_request() {
		static const int cols = 10;
		int id = r[0] & 0xff;
		fprintf(stderr, "xv65: request %d:%s:\n", id, req_id_to_string(id));
		int nn = ((n + (cols - 1)) / cols) * cols;
		for (int i = 0; i < nn;) {
			for (int j = 0; j < cols; j++, i++) {
				int c = r[i] & 0xff;
				if (i < n)
					fprintf(stderr, "%03d ", c);
				else
					fprintf(stderr, "    ");
			}
			i -= cols;
			for (int j = 0; j < cols; j++, i++) {
				int c = r[i] & 0xff;
				if (i < n)
					fprintf(stderr, "%c", isprint(c) ? c : '.');
				else
					fprintf(stderr, " ");
			}
			fprintf(stderr, "\n");
		}
	}
	void request() {
		int ret = ERR;
		r = req_buf.get_data();
		n = req_buf.get_length();
		int id = -1;
		if (n > 0)
			id = *r;
		if (v_TRCLEVEL > 1)
			dump_request();
		switch (id) {
		case REQ_FORK:
			ret = req_FORK();
			break;
		case REQ_EXIT:
			ret = req_EXIT();
			break;
		case REQ_WAIT:
			ret = req_WAIT();
			break;
		case REQ_PIPE:
			ret = req_PIPE();
			break;
		case REQ_KILL:
			ret = req_KILL();
			break;
		case REQ_GETPID:
			ret = req_GETPID();
			break;
		case REQ_SLEEP:
			ret = req_SLEEP();
			break;
		case REQ_EXEC:
			ret = req_EXEC();
			break;
		case REQ_FSTAT:
			ret = req_FSTAT();
			break;
		case REQ_OPEN:
			ret = req_OPEN();
			break;
		case REQ_READ:
		case REQ_WRITE:
			ret = req_rdwr(id);
			break;
		case REQ_CLOSE:
			ret = req_CLOSE();
			break;
		case REQ_DUP:
			ret = req_DUP();
			break;
		case REQ_CHDIR:
		case REQ_MKDIR:
		case REQ_UNLINK:
		case REQ_RMDIR:
			ret = req_filename(id);
			break;
		case REQ_ARGC:
			ret = req_ARGC();
			break;
		case REQ_ARGV:
			ret = req_ARGV();
			break;
		case REQ_ENV:
			ret = req_ENV();
			break;
		case REQ_TIME:
			ret = req_TIME();
			break;
		case REQ_LSEEK:
			ret = req_LSEEK();
			break;
		}
		v_REQRES = ret;
		if (ret != 0) {
			v_REQERRNO = ret;
			if (v_TRCLEVEL == 1 || v_ERREXIT)
				dump_request();
			if (v_TRCLEVEL || v_ERREXIT)
				fprintf(stderr, "xv65: failed %d:%s.\n", ret, errno_to_string(ret));
			if (v_ERREXIT)
				exit(1);
		} else if (v_TRCLEVEL > 1) {
			fprintf(stderr, "xv65: succeeded.\n");
		}
		req_buf.reset();
	}
	void put_REQPTRLO(int value) {
		int addr = (v_REQPTRHI << 8) | value;
		int len = m_get_uint16(addr);
		addr += 2;
		req_buf.reset();
		for (int i = 0; i < len; i++)
			req_buf.append_char(m_get_uint8(addr++));
		if (!len)
			req_buf.append_char(0);
		request();
	}
	void put_FRMDRAW() {
		int fps4 = v_FRMFPS;
		if (microio) {
			if (microio_refresh) {
				printf(STR_ESC_NORMAL);
				printf(STR_ESC_CLEAR);
				microio_refresh = false;
			}
			printf(STR_ESC_HOME);
			for (int i = 0; i < MicroIODisplay::N_OF_LINES; i++)
				printf("%s\n", display.get_line(i));
			if (!fps4)
				fps4 = 40;
		} else {
			microio_refresh = true;
		}
		fflush(stdout);
		if (fps4) {
			int ms = (int)(1000.0 / (fps4 / 4.0));
			struct timespec ts;
			ts.tv_sec = ms / 1000;
			ts.tv_nsec = 1000000 * (ms % 1000);
			nanosleep(&ts, NULL);
		}
	}
	int get_consize(int address) {
		int col = 80, row = 24;
#if !defined(ANDROID) && !defined(__ANDROID__)
		struct winsize win;
		if (ioctl(0, TIOCGWINSZ, &win) >= 0) {
			col = win.ws_col;
			row = win.ws_row;
		}
#endif
		return ((address == CONCOLS) ? col : row) & 0xff;
	}
	void set_CONESC(int value) {
		switch (value) {
		case ESC_HOME:
			printf(STR_ESC_HOME);
			break;
		case ESC_CLEAR:
			printf(STR_ESC_CLEAR);
			break;
		case ESC_NORMAL:
			printf(STR_ESC_NORMAL);
			break;
		default:
			if ((value >= ESC_BG_BLACK && value <= ESC_BG_WHITE) ||
			    (value >= ESC_FG_BLACK && value <= ESC_FG_WHITE))
				printf("\x1b[%dm", value);
		}
	}
public:
	Xv65Device() {
		signal(SIGALRM, sig_handler);
	}
	// IO
	void set_address_space(AddressSpace *dma) {
		m = dma;
	}
	void reset() {
		req_buf.reset();
		random.reset();
		display.reset();
		microio = false;
		microio_refresh = true;
		v_REQRES = 0;
		v_REQPTRHI = 0;
		v_REQERRNO = 0;
		v_FRMFPS = 0;
		v_TRCLEVEL = 0;
		v_ERREXIT = 0;
		putfp = stdout;
		memset(v_REQDAT, 0, sizeof(v_REQDAT));
	}
	void put(int address, int value) {
		address += IO_BASE;
		value &= 0xff;
		switch (address) {
		case PUTCHAR:
			fputc(value, putfp);
			microio = false;
			break;
		case REQPUT:
			req_buf.append_char(value);
			break;
		case REQEND:
			request();
			break;
		case REQPTRHI:
			v_REQPTRHI = value;
			break;
		case REQPTRLO:
			put_REQPTRLO(value);
			break;
		case REQERRNO:
			v_REQERRNO = value;
			break;
		case FRMFPS:
			v_FRMFPS = value;
			break;
		case FRMDRAW:
			put_FRMDRAW();
			break;
		case PUTUINT8:
			fprintf(putfp, "%d", value);
			microio = false;
			break;
		case RANDOM:
			random.put(value);
			break;
		case CONESC:
			set_CONESC(value);
			microio = false;
			break;
		case TRCLEVEL:
			v_TRCLEVEL = value;
			break;
		case ERREXIT:
			v_ERREXIT = value ? 0xff : 0;
			break;
		case REDIRPUT:
			if (value == 1)
				putfp = stdout;
			else if (value == 2)
				putfp = stderr;
			break;
		default:
			if (address >= REQDAT && address < REQDAT + 16) {
				v_REQDAT[address - REQDAT] = value;
			} else if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE) {
				display.put(address - CONVIDEO, value);
				microio = true;
			} else {
				if (v_TRCLEVEL > 1 || v_ERREXIT)
					fprintf(stderr, "xv65: io.put(%d,%d) unmapped.\n", address - IO_BASE, value);
				if (v_ERREXIT)
					exit(1);
			}
			break;
		}
	}
	int get(int address) {
		address += IO_BASE;
		switch (address) {
		case REQRES:
			return v_REQRES;
		case REQPTRHI:
			return v_REQPTRHI;
		case REQERRNO:
			return v_REQERRNO;
		case PIDSIZE:
			return sizeof(pid_t);
		case FRMFPS:
			return v_FRMFPS;
		case RANDOM:
			return random.get();
			break;
		case CONCOLS:
		case CONROWS:
			return get_consize(address);
		case TRCLEVEL:
			return v_TRCLEVEL;
		case ERREXIT:
			return v_ERREXIT;
		case REDIRPUT:
			if (putfp == stdout)
				return 1;
			else
				return 2;
		default:
			if (address >= REQDAT && address < REQDAT + 16) {
				return v_REQDAT[address - REQDAT];
			} else if (address >= CONVIDEO && address < CONVIDEO + MicroIODisplay::CONVIDEO_SIZE) {
				return display.get(address - CONVIDEO);
			} else {
				if (v_TRCLEVEL > 1 || v_ERREXIT)
					fprintf(stderr, "xv65: io.get(%d) unmapped.\n", address - IO_BASE);
				if (v_ERREXIT)
					exit(1);
			}
			return 0;
		}
	}
	// Device
	void set_args(int argc_, char **argv_) {
		argc = argc_;
		argv = argv_;
	}
	bool update(int status) {
		return false;
	}
};

Device *new_Device() {
	return new Xv65Device();
}

DeviceEntry entry("xv65", new_Device);

} // namespace
