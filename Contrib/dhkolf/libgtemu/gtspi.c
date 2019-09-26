/*

    This file is part of libgtemu, a library for Gigatron emulation.
    Copyright (C) 2019 David Heiko Kolf

    Published under the BSD-2-Clause license.
    https://opensource.org/licenses/BSD-2-Clause

*/

#include "gtemu.h"

enum CardState {
	C_IDLE = 0,
	C_RECVBIT1,
	C_RECVCMDNR,
	C_RECVARG,
	C_RECVCHECKSUM,
	C_SENDRESPONSE,
	C_WAITREAD,
	C_SENDIFCOND,
	C_SENDOCR,
	C_SENDSTART,
	C_SENDDATA,
	C_SENDCRC,
};

enum SDCardCmds {
	CMD0_GO_IDLE_STATE = 0,
	CMD8_SEND_IF_COND = 8,
	CMD16_SET_BLOCKLEN = 16,
	CMD17_READ_SINGLE_BLOCK = 17,
	CMD55_APP_CMD = 55,
	CMD58_READ_OCR = 58,
	ACMD41_SD_SEND_OP_COND = 41,
};

static const unsigned short crc16table[] = {
	0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50A5,  0x60C6,  0x70E7,
	0x8108,  0x9129,  0xA14A,  0xB16B,  0xC18C,  0xD1AD,  0xE1CE,  0xF1EF,
	0x1231,  0x0210,  0x3273,  0x2252,  0x52B5,  0x4294,  0x72F7,  0x62D6,
	0x9339,  0x8318,  0xB37B,  0xA35A,  0xD3BD,  0xC39C,  0xF3FF,  0xE3DE,
	0x2462,  0x3443,  0x0420,  0x1401,  0x64E6,  0x74C7,  0x44A4,  0x5485,
	0xA56A,  0xB54B,  0x8528,  0x9509,  0xE5EE,  0xF5CF,  0xC5AC,  0xD58D,
	0x3653,  0x2672,  0x1611,  0x0630,  0x76D7,  0x66F6,  0x5695,  0x46B4,
	0xB75B,  0xA77A,  0x9719,  0x8738,  0xF7DF,  0xE7FE,  0xD79D,  0xC7BC,
	0x48C4,  0x58E5,  0x6886,  0x78A7,  0x0840,  0x1861,  0x2802,  0x3823,
	0xC9CC,  0xD9ED,  0xE98E,  0xF9AF,  0x8948,  0x9969,  0xA90A,  0xB92B,
	0x5AF5,  0x4AD4,  0x7AB7,  0x6A96,  0x1A71,  0x0A50,  0x3A33,  0x2A12,
	0xDBFD,  0xCBDC,  0xFBBF,  0xEB9E,  0x9B79,  0x8B58,  0xBB3B,  0xAB1A,
	0x6CA6,  0x7C87,  0x4CE4,  0x5CC5,  0x2C22,  0x3C03,  0x0C60,  0x1C41,
	0xEDAE,  0xFD8F,  0xCDEC,  0xDDCD,  0xAD2A,  0xBD0B,  0x8D68,  0x9D49,
	0x7E97,  0x6EB6,  0x5ED5,  0x4EF4,  0x3E13,  0x2E32,  0x1E51,  0x0E70,
	0xFF9F,  0xEFBE,  0xDFDD,  0xCFFC,  0xBF1B,  0xAF3A,  0x9F59,  0x8F78,
	0x9188,  0x81A9,  0xB1CA,  0xA1EB,  0xD10C,  0xC12D,  0xF14E,  0xE16F,
	0x1080,  0x00A1,  0x30C2,  0x20E3,  0x5004,  0x4025,  0x7046,  0x6067,
	0x83B9,  0x9398,  0xA3FB,  0xB3DA,  0xC33D,  0xD31C,  0xE37F,  0xF35E,
	0x02B1,  0x1290,  0x22F3,  0x32D2,  0x4235,  0x5214,  0x6277,  0x7256,
	0xB5EA,  0xA5CB,  0x95A8,  0x8589,  0xF56E,  0xE54F,  0xD52C,  0xC50D,
	0x34E2,  0x24C3,  0x14A0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
	0xA7DB,  0xB7FA,  0x8799,  0x97B8,  0xE75F,  0xF77E,  0xC71D,  0xD73C,
	0x26D3,  0x36F2,  0x0691,  0x16B0,  0x6657,  0x7676,  0x4615,  0x5634,
	0xD94C,  0xC96D,  0xF90E,  0xE92F,  0x99C8,  0x89E9,  0xB98A,  0xA9AB,
	0x5844,  0x4865,  0x7806,  0x6827,  0x18C0,  0x08E1,  0x3882,  0x28A3,
	0xCB7D,  0xDB5C,  0xEB3F,  0xFB1E,  0x8BF9,  0x9BD8,  0xABBB,  0xBB9A,
	0x4A75,  0x5A54,  0x6A37,  0x7A16,  0x0AF1,  0x1AD0,  0x2AB3,  0x3A92,
	0xFD2E,  0xED0F,  0xDD6C,  0xCD4D,  0xBDAA,  0xAD8B,  0x9DE8,  0x8DC9,
	0x7C26,  0x6C07,  0x5C64,  0x4C45,  0x3CA2,  0x2C83,  0x1CE0,  0x0CC1,
	0xEF1F,  0xFF3E,  0xCF5D,  0xDF7C,  0xAF9B,  0xBFBA,  0x8FD9,  0x9FF8,
	0x6E17,  0x7E36,  0x4E55,  0x5E74,  0x2E93,  0x3EB2,  0x0ED1,  0x1EF0
};

static short updatecrc16 (short crc, unsigned char d) {
	return (crc << 8) ^ crc16table[((crc >> 8) ^ d) & 0xff];
}

static int handlecmd (struct GTPeriph *ph)
{
	switch (ph->sdcard.cmd) {
	case CMD0_GO_IDLE_STATE:
		ph->sdcard.idle = 1;
		return 1;
	case CMD8_SEND_IF_COND:
		return 1;
	case CMD16_SET_BLOCKLEN:
		return ph->sdcard.arg == 512;
	case CMD17_READ_SINGLE_BLOCK:
		ph->sdcard.addr = ph->sdcard.arg * 512;
		ph->sdcard.pendingread = 1;
		ph->sdcard.crc16 = 0;
		return 1;
	case CMD55_APP_CMD:
		ph->sdcard.appcmd = 1;
		return 1;
	case CMD58_READ_OCR:
		return 1;
	default:
		return 0;
	}
}

static int handleappcmd (struct GTPeriph *ph)
{
	switch (ph->sdcard.cmd) {
	case ACMD41_SD_SEND_OP_COND:
		ph->sdcard.idle = 0;
		return 1;
	default:
		return 0;
	}
}

static void spi0cardclock (struct GTState *gt, struct GTPeriph *ph)
{
	unsigned short mosi = (gt->expandercontrol & 0x8000) >> 15;
	switch (ph->sdcard.state) {
	case C_IDLE:
		gt->miso = 1;
		if (!mosi) {
			ph->sdcard.state = C_RECVBIT1;
		}
		break;
	case C_RECVBIT1:
		if (mosi) {
			ph->sdcard.state = C_RECVCMDNR;
			ph->sdcard.cmd = 0;
			ph->sdcard.bitcount = 0;
		} else {
			ph->sdcard.state = C_IDLE;
		}
		break;
	case C_RECVCMDNR:
		ph->sdcard.cmd = ph->sdcard.cmd << 1 | mosi;
		if (ph->sdcard.bitcount < 5) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.arg = 0;
			ph->sdcard.bitcount = 0;
			ph->sdcard.state = C_RECVARG;
		}
		break;
	case C_RECVARG:
		ph->sdcard.arg = ph->sdcard.arg << 1 | mosi;
		if (ph->sdcard.bitcount < 31) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.checksum = 0;
			ph->sdcard.bitcount = 0;
			ph->sdcard.state = C_RECVCHECKSUM;
		}
		break;
	case C_RECVCHECKSUM:
		ph->sdcard.checksum = ph->sdcard.checksum << 1 | mosi;
		if (ph->sdcard.bitcount < 7) {
			ph->sdcard.bitcount++;
		} else {
			if (ph->sdcard.appcmd) {
				ph->sdcard.appcmd = 0;
				ph->sdcard.invalidcmd = !handleappcmd(ph);
			} else {
				ph->sdcard.invalidcmd = !handlecmd(ph);
			}
			ph->sdcard.state = C_SENDRESPONSE;
			ph->sdcard.bitcount = 0;
		}
		break;
	case C_SENDRESPONSE:
		if (ph->sdcard.bitcount < 7) {
			if (ph->sdcard.bitcount == 5) {
				gt->miso = ph->sdcard.invalidcmd ? 1 : 0;
			} else {
				gt->miso = 0;
			}
			ph->sdcard.bitcount++;
		} else {
			gt->miso = ph->sdcard.idle ? 1 : 0;
			ph->sdcard.bitcount = 0;
			switch (ph->sdcard.cmd) {
			case CMD8_SEND_IF_COND:
				ph->sdcard.state = C_SENDIFCOND;
				break;
			case CMD58_READ_OCR:
				ph->sdcard.state = C_SENDOCR;
				break;
			case CMD17_READ_SINGLE_BLOCK:
				ph->sdcard.state = C_WAITREAD;
				break;
			default:
				ph->sdcard.state = C_IDLE;
				break;
			}
		}
		break;
	case C_SENDIFCOND:
		if (ph->sdcard.bitcount < 23) {
			gt->miso = 0;
		} else {
			gt->miso = ph->sdcard.arg & (0x80 >> (ph->sdcard.bitcount - 24)) ? 1 : 0;
		}
		if (ph->sdcard.bitcount < 31) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.state = C_IDLE;
		}
		break;
	case C_SENDOCR:
		gt->miso = 0;
		if (ph->sdcard.bitcount < 31) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.state = C_IDLE;
		}
		break;
	case C_WAITREAD:
		gt->miso = 1;
		if (ph->sdcard.bitcount < 7) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.bitcount = 0;
			if (!ph->sdcard.pendingread) {
				ph->sdcard.state = C_SENDSTART;
			}
		}
		break;
	case C_SENDSTART:
		if (ph->sdcard.bitcount < 7) {
			gt->miso = 1;
			ph->sdcard.bitcount++;
		} else {
			gt->miso = 0;
			ph->sdcard.bitcount = 0;
			ph->sdcard.state = C_SENDDATA;
		}
		break;
	case C_SENDDATA:
		gt->miso = ph->sdcard.readdata[0] & (0x80 >> ph->sdcard.bitcount) ? 1 : 0;
		if (ph->sdcard.bitcount < 7) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.bitcount = 0;
			ph->sdcard.crc16 = updatecrc16(ph->sdcard.crc16,
				ph->sdcard.readdata[0]);
			if (ph->sdcard.remainingdata > 1) {
				ph->sdcard.readdata++;
				ph->sdcard.remainingdata--;
			} else {
				ph->sdcard.state = C_SENDCRC;
			}
		}
		break;
	case C_SENDCRC:
		gt->miso = ph->sdcard.crc16 & (0x8000 >> ph->sdcard.bitcount) ? 1 : 0;
		if (ph->sdcard.bitcount < 15) {
			ph->sdcard.bitcount++;
		} else {
			ph->sdcard.state = C_IDLE;
		}
		break;
	}
}

void gtspi_onrisingsclk (struct GTState *gt, struct GTPeriph *ph)
{
	unsigned short ssx = gt->expandercontrol & 0x003c;
	if ((ssx & 4) == 0) {
		spi0cardclock(gt, ph);
	}
}

int gtspi_pollread (struct GTPeriph *ph, size_t *addr)
{
	if (ph->sdcard.pendingread) {
		*addr = ph->sdcard.addr;
		return 1;
	}
	return 0;
}

void gtspi_providereadbuffer (struct GTPeriph *ph,
	const unsigned char *buffer)
{
	ph->sdcard.readdata = buffer;
	ph->sdcard.remainingdata = 512;
	ph->sdcard.pendingread = 0;
}

int gtspi_readbufferinuse (struct GTPeriph *ph)
{
	switch (ph->sdcard.state) {
	case C_WAITREAD:
		return !ph->sdcard.pendingread;
	case C_SENDSTART:
	case C_SENDDATA:
		return 1;
	default:
		return 0;
	}
}

