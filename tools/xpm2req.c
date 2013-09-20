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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "input.xpm"
char *line()
{
	static int i = 0;
	return input_xpm[i++];
}

char lookup[100];

int main(int argc, char *argv[])
{
	int i, n, width, height, cols, ext;

	n = sscanf(line(), "%d %d %d %d", &width, &height, &cols, &ext);
	assert(n == 4);
	assert(cols <= 16);
	printf("\t; %dx%d, %d colors - converted by xpm2req\n", width, height, cols);
	printf("\t%d\n", cols - 1);
	for (i = 0; i < cols; i++) {
		char *s = line();
		n = strlen(s);
		assert((i == 0 && n == 8) || (i != 0 && n == 11));
		lookup[i] = s[0];
		if (i == 0) {
			printf("\t  0   0   0 ; UNUSED\n");
		} else {
			int c = strtol(&s[11 - 6], NULL, 16);
			printf("\t%3d %3d %3d ; %c\n", c >> 16, (c >> 8) & 0xff, c & 0xff, s[0]);
		}
	}
	lookup[i] = 0;
	printf("\t.lookup \"%s\"\n", lookup);
	for (i = 0; i < height; i++)
		printf("\t.bits \"%s\"\n", line());
	return 0;
}
