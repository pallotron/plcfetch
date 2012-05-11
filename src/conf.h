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

#ifndef CONF_H
#define CONF_H

#define APPNAME          "plcfetch"
#define VERSION          "0.0.1"

#define DEFAULT_PORT     "/dev/ttyS0"
#define DEFAULT_BAUDRATE "9600"
#define DEFAULT_DATABITS "8"
#define DEFAULT_PARITY   "N"
#define DEFAULT_STOPBIT  "1"

#define DEFAULT_CFGFILE  "/.plcfetchrc"

#define MAX_CHARS        500

// codes to use into the database
#define MARCIA                 1
#define ARRESTO                2
#define ROTTURA_FILO           3
#define ROTTURA_BALLERINO      4
#define RESET_CONTACOLPI_ISOLA 5
#define FINE_CONTEGGIO_ISOLA   6
#define SOGLIA_TENSIONE  	   10
#define SOGLIA_CORRENTE        11
#define STAMPA_ISOLA           12
#define START_PLCFETCH         20


// seconds value for each update instruction
#define UPDATE_FREQ            10

#include "plcfetch.h"
#include <string.h>
#include <time.h>

typedef struct {

	char *port;
	char *baudrate;
	char *databits;
	char *parity;
	char *stopbit;

	char *dsn;
	char *dbuser;
	char *dbpass;

	char *cfgfile;

	int verbose;
	int daemon;
	int polling;

	int instancenum;
	int nosendstart;

	int update_secs;

} Configuration;

Configuration cfg;

typedef struct {

	int num_impianto;
	char *ip_macchina;
	float soglia_tensione, soglia_corrente;
	float k1;		// multiply coefficient for voltage
	float k2;		// multiply coefficient for current
	int num_isole;
	int array_isole[4];	// num_isola
	int array_cod_isole[4];	// cod_isola
	float array_island_k1[4];	// divisor coefficient for encoder counters

} Embedded_info;

Embedded_info emb_info;

int first_loop;
time_t curr_timestamp;

int parse_cfgfile(char *file);
void fill_default_options(void);
void free_cfg(void);
void free_emb_info(void);
int checkbaudrate(int baudrate);
int checkdatabits(int databits);
int checkparity(char parity);
int checkstopbits(int stop);
int checkinstancenum(int num);
int checkupdatesecs(int num);

#endif				/* CONF_H */
