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

#ifndef ODBC_H
#define ODBC_H

// unixODBC headers
#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include "serial.h"

SQLHENV sql_env;
SQLHDBC sql_hdbc;

void connect_db(char *DSN, char *user, char *pass);
void disconnect_db(void);
void get_embedded_infos(void);
void do_sql_things(datarecord d);
void odbc_alloc();
void odbc_free();

#endif				/* ODBC_H */
