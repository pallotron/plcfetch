/* This file is part of plcfetch
 * (c) Copyright 2006 Angelo Failla aka pallotron <pallotron@freaknet.org>
 *
 * it is based on conf.c from FreakNet MediaLab Netsukuku project
 * (c) Copyright 2005 Andrea Lo Pumo aka AlpT <alpt@freaknet.org>
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
 *
 * --
 * conf.c:
 * Configuration file loader and parser. All the accepted option. which are
 * listed in conf.h, are put in the environment for later retrievement.
 */

#include "conf.h"
#include "utils.h"
#include "includes.h"
#include <string.h>
#include <time.h>

int parse_cfgfile(char *file)
{

	return 0;

}

/* this function free the memory in cfg global struct */
void free_cfg(void)
{

/*	if( cfg.port     != 0) free(cfg.port);
	if( cfg.baudrate != 0) free(cfg.baudrate);
	if( cfg.databits != 0) free(cfg.databits);
	if( cfg.stopbit  != 0) free(cfg.stopbit);
	if( cfg.parity   != 0) free(cfg.parity);
	
	//if( cfg.cfgfile  != 0) free(cfg.cfgfile);
	
	if( cfg.dsn      != 0) free(cfg.dsn);
	if( cfg.dbuser   != 0) free(cfg.dbuser);
	if( cfg.dbpass   != 0) free(cfg.dbpass);*/
	free(cfg.port);
	free(cfg.baudrate);
	free(cfg.databits);
	free(cfg.stopbit);
	free(cfg.parity);

	//if( cfg.cfgfile  != 0) free(cfg.cfgfile);

	free(cfg.dsn);
	free(cfg.dbuser);
	free(cfg.dbpass);


}

/* this function free the memory in emb_info global struct */
void free_emb_info(void)
{

	if (emb_info.ip_macchina != 0)
		free(emb_info.ip_macchina);

}

/* this function fill the default value in cfg struct */
void fill_default_options(void)
{

	setzero(&cfg, sizeof(cfg));

	cfg.verbose = 0;
	cfg.daemon = 0;
	cfg.polling = 0;
	cfg.instancenum = 1;
	cfg.nosendstart = 0;
	cfg.update_secs = UPDATE_FREQ;

	cfg.baudrate = strndup(DEFAULT_BAUDRATE, sizeof(DEFAULT_BAUDRATE));
	cfg.databits = strndup(DEFAULT_DATABITS, sizeof(DEFAULT_DATABITS));
	cfg.port = strndup(DEFAULT_PORT, sizeof(DEFAULT_PORT));
	cfg.stopbit = strndup(DEFAULT_STOPBIT, sizeof(DEFAULT_STOPBIT));
	cfg.parity = strndup(DEFAULT_PARITY, sizeof(DEFAULT_PARITY));

	/*cfg.cfgfile  = strncat(getenv("HOME"),DEFAULT_CFGFILE,
	   strlen(getenv("HOME"))+strlen(DEFAULT_CFGFILE)); */

	setzero(&emb_info, sizeof(emb_info));
	curr_timestamp = time(NULL);

}

int checkbaudrate(int baudrate)
{

	switch (baudrate) {

	case 300:
	case 1200:
	case 2400:
	case 4800:
	case 9600:
	case 19200:
	case 38400:
	case 57600:
	case 115200:
	case 230400:
		return 1;
		break;

	default:
		return 0;
		break;
	}

}

int checkdatabits(int databits)
{

	return (databits >= 5 && databits <= 8) ? 1 : 0;

}

int checkparity(char parity)
{

	return (parity >= 'L' && parity <= 'P') ? 1 : 0;

}

int checkstopbits(int stop)
{

	return (stop == 1 || stop == 2) ? 1 : 0;

}

int checkinstancenum(int num)
{

	return (num > 0 && num <= 10) ? 1 : 0;

}

int checkupdatesecs(int num)
{

	return (num > 0) ? 1 : 0;

}
