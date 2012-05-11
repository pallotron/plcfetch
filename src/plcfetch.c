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

#include "includes.h"
#include "conf.h"
#include "utils.h"
#include "plcfetch.h"
#include "sysdep.h"
#include "serial.h"
#include "odbc.h"

#include <signal.h>

/* this function prints the usage message */
void usage(void)
{

	fprintf(stderr, "\nUsage:\n"
		"%s [-h] [-v] [-B] [-N] [-T]\n"
		"\t [-i DEV] [-b BAUDRATE] [-d DATABITS] [-p PARITY ] [-s STOPBIT]\n"
		"\t [-I IP] [-D ODBC_DSN] [-U DBUSER] [-P DBPASS] [-n INSTANCE_NUMBER]\n"
		"[-f CONF_FILE]\n"
		"\n"
		"-q                  Enable database configuration data polling\n"
		"-h                  Print this help\n"
		"-v                  Verbose/Debug mode on (default is off)\n"
		"-B                  Background/Daemon mode\n"
		"-N                  Do not send the START_PLCFETCH signal at startup\n"
		"-T SECONDS          Numbers of seconds between each update (default 10)\n"
		"\n"
		"-i DEVICE           Serial Port device (default is %s).\n"
		"-b BAUDRATE         Baud rate (default is %s)\n"
		"                    possible values are:\n"
		"                    300, 1200, 2400, 4800, 9600, 19200,\n"
		"                    38400, 57600, 115200, 230400 \n"
		"-d DATABITS         Data bits (default is %s)\n"
		"                    possible values are:\n"
		"                    5, 6, 7, 8\n"
		"-p PARITY           Parity (default is %s)\n"
		"                    possible values are:\n"
		"                    L, M, N, O, P\n"
		"-s STOPBIT          Stop bit (default is %s)\n"
		"                    possible values are:\n"
		"                    1, 2\n"
		"\n"
		"-I IP               IP address of the host\n"
		"-D ODBC_DSN         DSN Name of the unixODBC connection\n"
		"-U DBUSER           Database Username\n"
		"-P DBPASS           Password for DBUSER\n"
		"-n INSTANCE_NUMBER  An integer representing the instance\n"
		"                    number of plcfetch (default 1)\n"
		"\n"
		"-f CONF_FILE  Path for the configuration file (default is ~%s)\n"
		"\n",
		APPNAME,
		DEFAULT_PORT, DEFAULT_BAUDRATE, DEFAULT_DATABITS,
		DEFAULT_PARITY, DEFAULT_STOPBIT, DEFAULT_CFGFILE);

	exit(1);

}

/* this function parses the command line options */
void parse_opts(int argc, char *argv[])
{

	int c;

	while ((c =
		getopt(argc, argv, "T:I:i:n:b:d:p:s:H:D:U:P:f:hvBqN")) != EOF) {
		switch (c) {
		case 'h':
			usage();
			break;
		case 'v':
			cfg.verbose = 1;
			break;
		case 'B':
			cfg.daemon = 1;
			break;
		case 'q':
			cfg.polling = 1;
			break;
		case 'N':
			cfg.nosendstart = 1;
			break;
		case 'T':
			if (checkupdatesecs(atoi(optarg)))
				cfg.update_secs = atoi(optarg);
			else
				printFatal
				    ("Number of seconds must be greater then 0!");
			break;
		case 'n':
			if (checkinstancenum(atoi(optarg)))
				cfg.instancenum = atoi(optarg);
			else
				printFatal("Number of instance not valid!");
			break;
		case 'i':
			cfg.port = strndup(optarg, strlen(optarg));
			break;
		case 'I':
			emb_info.ip_macchina = strndup(optarg, strlen(optarg));
			break;
		case 'b':
			if (checkbaudrate(atoi(optarg)))
				cfg.baudrate = strndup(optarg, strlen(optarg));
			else {
				printFatal("Baud Rate value is not valid");
			}
			break;
		case 'd':
			if (checkdatabits(atoi(optarg))
			    && strlen(optarg) == 1)
				cfg.databits = strndup(optarg, strlen(optarg));
			else {
				printFatal("Data bits value is not valid");
			}
			break;
		case 'p':
			if (checkparity(optarg[0]))
				cfg.parity = strndup(optarg, strlen(optarg));
			else {
				printFatal("Parity value is not valid");
			}
			break;
		case 's':
			if (checkstopbits(atoi(optarg))
			    && strlen(optarg) == 1)
				cfg.stopbit = strndup(optarg, strlen(optarg));
			else {
				printFatal("Stop bit value is not valid");
			}
			break;
		case 'D':
			cfg.dsn = strndup(optarg, strlen(optarg));
			break;
		case 'U':
			cfg.dbuser = strndup(optarg, strlen(optarg));
			break;
		case 'P':
			cfg.dbpass = strndup(optarg, strlen(optarg));
			break;
		case 'f':
			cfg.cfgfile = strndup(optarg, strlen(optarg));
			break;
		default:
			usage();
			break;
		}
	}
}

/* this functions opens and configure the serial port */
void cfgport(void)
{

	if ((portfd = open(cfg.port, O_RDWR)) == -1) {
		printFatal("Unable to open serial port");
	}

	/* these functions, declared on sysdep1.c
	 * configure the serial port
	 * these functions are from minicom source */
	m_savestate(portfd);
	m_setparms(portfd, cfg.baudrate, cfg.parity, cfg.databits, 1, 0);
	m_nohang(portfd);
	m_hupcl(portfd, 1);
	m_flush(portfd);

}

/* this function free the memory allocated and exit
 * and it is called on signals */
void destroyall(int status)
{

	odbc_free();
	disconnect_db();
	close(portfd);
	free_emb_info();
	free_cfg();
	exit(-1);

}

/* this is the main body */
int main(int argc, char *argv[])
{

	if (cfg.verbose)
		printf("%s version %s\n", APPNAME, VERSION);

	/* fill the cfg struct with the default options */
	fill_default_options();
	/* parse options from config file */
	parse_cfgfile(cfg.cfgfile);
	/* parse the command line options */
	parse_opts(argc, argv);

	odbc_alloc();
	connect_db(cfg.dsn, cfg.dbuser, cfg.dbpass);
	get_embedded_infos();
	disconnect_db();

	/* if cfg.daemon option is set to 1 we go to background */
	if (cfg.daemon)
		daemon(0, 0);

	/* set signal hook function */
	signal(SIGHUP, &destroyall);
	signal(SIGINT, &destroyall);
	signal(SIGQUIT, &destroyall);
	signal(SIGTERM, &destroyall);
	signal(SIGABRT, &destroyall);

	/* open and configure the serial port */
	cfgport();
	/* enter the main loop */
	serial_mainloop();

	odbc_free();
	disconnect_db();
	free_cfg();
	free_emb_info();

	exit(0);
}
