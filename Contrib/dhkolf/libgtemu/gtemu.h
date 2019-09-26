/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

    Version 0.1.0

*/

#ifndef GTEMU_H
#define GTEMU_H 1

#include <stddef.h>

#define GT_SCREENWIDTH 160
#define GT_SCREENHEIGHT 480

struct GTRomEntry {
	unsigned char i;
	unsigned char d;
};

/*
GTState containes the state of the CPU and its fields are open for
inspection and manipulation.
*/
struct GTState {
	int pc;
	unsigned char ir, d, ac, x, y, out, in;
	struct GTRomEntry *rom;
	size_t romcount;
	unsigned char *ram;
	size_t rammask;
	int hasexpander;
	unsigned short expandercontrol;
	unsigned char miso;
};

/*
GTPeriph contains the state of the peripherals; the board, the video
output, the audio output, the GT1 loader and the serial output.  The
variables are implementation details and should not be accessed directly.
*/
struct GTPeriph {
	struct {
		int booted;
		unsigned char xout, undef;
		unsigned long long clock;
	} board;
	struct {
		int x, y;
	} video;
	struct {
		unsigned long long sampleticks1k, sampletickscount1k;
		unsigned short sample;
	} audio;
	struct {
		int state;
		int bitcount;
		unsigned char remainingframe;
		unsigned char remainingfiller;
		unsigned short addr;
		unsigned char checksum;
		const unsigned char *data;
		size_t remainingdata, remainingsegment;
		char prevkey;
	} loader;
	struct {
		int invpulse, sentfull;
		int count, bitcount;
		unsigned char outchar;
		char *buffer;
		size_t *bufferpos, buffersize;
	} serialout;
	struct {
		int state;
		int idle, pendingread;
		int appcmd, invalidcmd;
		int bitcount;
		unsigned char cmd, checksum;
		unsigned long arg;
		size_t addr;
		const unsigned char *readdata;
		size_t remainingdata;
		unsigned short crc16;
	} sdcard;
};

/* functions defined in gtemu.u */

extern void gtemu_init (struct GTState *gt,
	struct GTRomEntry *rom, size_t romsize,
	unsigned char *ram, size_t ramsize);

extern void gtemu_initperiph (struct GTPeriph *ph, int audiofreq,
	unsigned long randseed);

unsigned long gtemu_randomizemem (unsigned long seed,
	void *mem, size_t size);

extern unsigned long long gtemu_getclock (struct GTPeriph *ph);

extern unsigned char gtemu_getxout (struct GTPeriph *ph);

extern int gtemu_processtick (struct GTState *gt, struct GTPeriph *ph);

extern int gtemu_processscreen (struct GTState *gt, struct GTPeriph *ph,
	void *pixels, int pitch,
	unsigned short *samples, size_t maxsamples, size_t *nsamples);

void gtemu_placelights (struct GTPeriph *ph, void *pixels, int pitch,
	int power);

void gtserialout_setbuffer (struct GTPeriph *ph, char *buffer,
	size_t buffersize, size_t *bufferpos);

/* functions defined in gtloader.c */

extern int gtloader_sendgt1 (struct GTPeriph *ph,
	const char *data, size_t datasize);

extern int gtloader_isactive (struct GTPeriph *ph);

extern int gtloader_validategt1 (const char *data, size_t datasize);

extern int gtloader_sendtext (struct GTPeriph *ph,
	const char *data, size_t datasize);

extern int gtloader_sendkey (struct GTState *gt, struct GTPeriph *ph,
	char key);

/* functions defined in gtspi.c */

extern int gtspi_pollread (struct GTPeriph *ph, size_t *addr);

extern void gtspi_providereadbuffer (struct GTPeriph *ph,
	const unsigned char *buffer);

extern int gtspi_readbufferinuse (struct GTPeriph *ph);

#endif /* GTEMU_H */

