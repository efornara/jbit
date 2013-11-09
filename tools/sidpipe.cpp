/*
    
    sidpipe
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

A simple pipe interface to the SID library reSID:
http://www.zimmers.net/anonftp/pub/cbm/crossplatform/emulators/resid/index.html

Tested with resid-0.16.tar.gz on a debian stable.

Compile with: g++ -O2 -s -o sidpipe sidpipe.cpp -lresid -lasound

See sidtype for an example of how to use it.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>

#include <resid/sid.h>
#include <alsa/asoundlib.h>

#define IN_BUF_SIZE 512
#define OUT_BUF_SIZE 512

#define SID_SAMPLE_RATE 44100
#define SID_CLOCK 985248

#define LOOP_FREQ 100

namespace {

void fatal(const char *format, ...) {
	va_list ap;

	va_start(ap, format);
	fprintf(stderr, "sidpipe: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

signed short out_buf[OUT_BUF_SIZE];
unsigned char in_buf[IN_BUF_SIZE];

const char *device = "default";
snd_pcm_t *handle;

void sound_open() {
	int err;

	if ((err = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
		fatal("sound_open failed: %s", snd_strerror(err));
	if ((err = snd_pcm_set_params(handle, SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED, 1, SID_SAMPLE_RATE, 1, 500000)) < 0)
		fatal("sound_open failed: %s", snd_strerror(err));
}

void sound_write(int n) {
	snd_pcm_sframes_t frames = snd_pcm_writei(handle, out_buf, n);
	if (frames < 0)
		frames = snd_pcm_recover(handle, frames, 0);
	if (frames < 0)
		fprintf(stderr, "sidpipe: snd_pcm_writei failed\n");
	else if (frames > 0 && frames < n)
		fprintf(stderr, "sidpipe: short write (expected %i, wrote %li)\n", n, frames);
}

void sound_emit(SID &sid, cycle_count delta_t) {
	while (delta_t) {
		int n = sid.clock(delta_t, out_buf, OUT_BUF_SIZE);
		sound_write(n);
	}
}

void sound_close() {
	snd_pcm_close(handle);
}

} // namespace

int main(int argc, char *argv[]) {
	SID sid;
	sound_open();
	sid.reset();
	if (fcntl(0, F_SETFL, O_NONBLOCK) != 0)
		fatal("fcntl failed: %d", errno);
	int i = 0;
	while (1) {
		int n = read(0, &in_buf[i], sizeof(in_buf) - i);
		if (n < 0) {
			if (errno != EAGAIN && errno != EWOULDBLOCK)
				fatal("read failed: %d", errno);
			sound_emit(sid, SID_CLOCK / LOOP_FREQ);
		} else if (n > 0) {
			n += i;
			for (i = 0; i < n; i += 2) {
				if (n - i >= 2)
					sid.write(in_buf[i], in_buf[i + 1]);
			}
			if (n & 1)
				i = 1;
		} else {
			break;
		}
	}
	sound_close();
}
