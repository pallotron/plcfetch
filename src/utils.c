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
 * 
 * --
 * utils.c:
 * utilities functions and macros
 */

#include "utils.h"
#include "conf.h"
#include "includes.h"

void printDebug(char *msg)
{
	fprintf(stdout, "[D] %s-%s Debug >> %s\n", APPNAME, VERSION, msg);
}

void printInfo(char *msg)
{
	fprintf(stdout, "[I] %s-%s Info >> %s\n", APPNAME, VERSION, msg);
}

void printError(char *msg)
{
	fprintf(stdout, "[E] %s-%s Error >> %s\n", APPNAME, VERSION, msg);
}

void printFatal(char *msg)
{
	fprintf(stdout, "[F] %s-%s Fatal >> %s\n", APPNAME, VERSION, msg);
	destroyall(-1);
}
