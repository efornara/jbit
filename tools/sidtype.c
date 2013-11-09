/*
    
    sidtype
    Copyright (C) 2013  Emanuele Fornara

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

/*

Assuming that sidpipe is in your path, you can start sidtype and type:

    0 0
    1 22
    5 0
    6 240
    4 17
    24 15

After the last line, you should start hearing a tone.

You can also use the register identifiers (shown when sidtype starts).
For example, to mute the tone, type (case insensitive):

    MODE_VOL 0

Exit with Ctrl-D.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <unistd.h>

const char *sid_regs[] = {
	"FREQLO1",
	"FREQHI1",
	"PWLO1",
	"PWHI1",
	"CR1",
	"AD1",
	"SR1",
	"FREQLO2",
	"FREQHI2",
	"PWLO2",
	"PWHI2",
	"CR2",
	"AD2",
	"SR2",
	"FREQLO3",
	"FREQHI3",
	"PWLO3",
	"PWHI3",
	"CR3",
	"AD3",
	"SR3",
	"FCLO",
	"FCHI",
	"RES_FILT",
	"MODE_VOL",
	0
};

int sid_fd;

void sid_open() {
	int p[2];
	pipe(p);
	if (fork() == 0) {
		char *argv[3];
		argv[0] = strdup("env");
		argv[1] = strdup("sidpipe");
		argv[2] = NULL;
		close(0);
		dup(p[0]);
		close(p[0]);
		close(p[1]);
		execvp("/usr/bin/env", argv);
		printf("sidtype: cannot exec sidpipe\n");
		exit(1);
	} else {
		sid_fd = p[1];
		close(p[0]);
	}
} 

unsigned char cmd[2];
char line[256];

int process_line() {
	if (line[0] == '#' || line[0] == '\n')
		return 0;
	const char *reg_s = strtok(line, "\t\n ");
	if (!reg_s)
		return printf("error: expected register.\n");
	const char *val_s = strtok(NULL, "\t\n ");
	if (!val_s)
		return printf("error: expected value.\n");
	const char *unexpected_s = strtok(NULL, "\t\n ");
	if (unexpected_s)
		return printf("error: unexpected trailing chars.\n");
	int reg = -1;
	if (isdigit(reg_s[0])) {
		reg = atoi(reg_s);
	} else {
		for (reg = 0; sid_regs[reg]; reg++)
			if (!strcasecmp(reg_s, sid_regs[reg]))
				break;
	}
	if (reg < 0 || reg > 24)
		return printf("error: register: 0..24 or id.\n");
	int val = atoi(val_s);
	if (val < 0 || val > 255)
		return printf("error: value: 0..255.\n");
	cmd[0] = reg;
	cmd[1] = val;
	write(sid_fd, cmd, sizeof(cmd));
	printf("sent %s %d\n", sid_regs[reg], val);
	return 0;
}

int main(int argc, char *argv[]) {
	int i;
	sid_open();
	printf("line format: register value\nregister ids:");
	for (i = 0; sid_regs[i]; i++)
		printf(" %s", sid_regs[i]);
	printf("\n\n");
	while (1) {
		int n = read(0, line, sizeof(line));
		if (n == 0)
			return 0;
		if (n < 0)
			return 1;
		if (n == sizeof(line)) {
			printf("line too long.\n");
			return 1;
		}
		line[n] = 0;
		process_line();
	}
}
