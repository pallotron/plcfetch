/* This file is part of plcfetch
 * (c) Copyright 2006 Angelo Failla aka pallotron <pallotron@freaknet.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef SERIAL_H
#define SERIAL_H

typedef struct {
	unsigned char start;	// 0x01 start code (SOH)
	int length;		// 0x18 == 24 elements
	unsigned char cmd;	// 0x04 data ready
	unsigned char databytes[24];	// array of "length" size, to be malloc,
	// on response the first byte can be 
	// 0x06 for ack, 0x21 fornack
	float voltage;
	int current;

	float encoder[4];

	unsigned char checksum;	// xor BCC checksum
	unsigned char end;	// 0x03 end code (ETX)
} datarecord;

#ifndef SOH
#define SOH ('\001')
#endif

#ifndef ETX
#define ETX ('\003')
#endif

#ifndef ACK
#define ACK (0x06)
#endif

#ifndef NACK
#define NACK (0x15)
#endif

/* this macro convert 2 bytes from an unsigned char array into a short int */
#define FETCH2BYTES(msb, lsb, array) (((array[msb] << 8) & 0xff00) | array[lsb])
/* this macro convert 4 bytes from an unsigned char array into a long int */
/* start is the first bytes of the stream, NOT the msb byte */
#define FETCH4BYTES(array, start) (((unsigned long)array[start+3] << 24) | ((unsigned long)array[start+2] << 16) | ((unsigned long)array[start+1] << 8) | ((unsigned long)array[start]))
//
/* this macro take n bits from x starting from p position */
#define GETBITS(x, p, n) (x >> (p+1-n)) & ~(~0 << n);

#define MAX_HEX_VALUE 1023
#define MAX_VOLT      10

void serial_mainloop(void);

#endif				/* SERIAL_H */
