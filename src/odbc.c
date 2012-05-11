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
 * odbc.c:
 * unixODBC functions and variables
 */

#include "utils.h"
#include "includes.h"
#include "odbc.h"
#include "conf.h"

int last_arresto = 0;
int last_marcia = 0;
int last_rotturafilo = 0;
int last_rotturaballerino = 0;
int last_sogliatensione = 0;
int last_sogliacorrente = 0;

int last_resetcontacolpi[4];
int last_fineconteggio[4];
int last_stampaisola[4];

float db_voltage = 0;
int db_current = 0;

void extract_error(char *fn, SQLHANDLE handle, SQLSMALLINT type)
{

	SQLINTEGER i = 0;
	SQLINTEGER native;
	SQLCHAR state[7];
	SQLCHAR text[256];
	SQLSMALLINT len;
	SQLRETURN ret;

	// TODO: improve logout and verbose mode use log utils from netsukuku
	fprintf(stderr, "\n"
		"The driver reported the following diagnostics whilst running "
		"%s\n\n", fn);

	do {
		ret =
		    SQLGetDiagRec(type, handle, ++i, state, &native, text,
				  sizeof(text), &len);
		// TODO: improve logout and verbose mode use log utils from netsukuku
		if (SQL_SUCCEEDED(ret))
			printf("%s:%ld:%ld:%s\n", state, i, native, text);
	} while (ret == SQL_SUCCESS);

}

void odbc_alloc()
{
	/* Allocate an environment handle */
	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sql_env);
	/* We want ODBC 3 support */
	SQLSetEnvAttr(sql_env, SQL_ATTR_ODBC_VERSION, (void *) SQL_OV_ODBC3, 0);
	/* Allocate a connection handle */
	SQLAllocHandle(SQL_HANDLE_DBC, sql_env, &sql_hdbc);

}

void odbc_free()
{
	SQLFreeHandle(SQL_HANDLE_DBC, sql_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, sql_env);
}

void connect_db(char *DSN, char *user, char *pass)
{

	SQLRETURN sql_ret;
	char connect_string[255];
	//extern SQLHENV        sql_env;
	//extern SQLHDBC        sql_hdbc;

	/* Connect to the DSN */
	snprintf(connect_string, 255, "DSN=%s;UID=%s;PWD=%s",
		 cfg.dsn, cfg.dbuser, cfg.dbpass);
	sql_ret = SQLDriverConnect(sql_hdbc, NULL, connect_string, SQL_NTS,
				   NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		// TODO: improve logout and verbose mode use log utils from netsukuku
		printf("Error SQLConnect %d\n", sql_ret);
		extract_error("SQLDriverConnect", sql_hdbc, SQL_HANDLE_DBC);
		//printFatal("Error connecting to DB\n");
		return;
	}
	// TODO: improve logout and verbose mode use log utils from netsukuku
	if (cfg.verbose)
		printf("Connected !\n");

}

void disconnect_db()
{

	//extern SQLHENV        sql_env;
	//extern SQLHDBC        sql_hdbc;

	SQLEndTran(SQL_HANDLE_ENV, sql_env, SQL_ROLLBACK);
	SQLDisconnect(sql_hdbc);
	if (cfg.verbose)
		printf("Disconnected !\n");

}

void get_embedded_infos(void)
{

	SQLHSTMT sql_stmt;
	SQLRETURN sql_ret;
	char query[500];
	SQLINTEGER signal, num_isola, cod_isola;
	float k1;
	int i = 0;
	int j = 0;

	sql_ret = SQLAllocHandle(SQL_HANDLE_STMT, sql_hdbc, &sql_stmt);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLAllocHandle %d\n", sql_ret);
		extract_error("SQLAllocHandle", sql_hdbc, SQL_HANDLE_DBC);
		//printFatal("Error during SQLAllocHandle\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	snprintf(query, 500, "SELECT num_impianto, ip_macchina, "
		 "soglia_tensione, soglia_corrente, k1, k2 "
		 "FROM dbo.AIMP_Principale WITH (NOLOCK) WHERE ip_macchina = '%s'",
		 emb_info.ip_macchina);

	sql_ret = SQLBindCol(sql_stmt, 1, SQL_C_SLONG,
			     &emb_info.num_impianto,
			     sizeof(emb_info.num_impianto), &signal);

	sql_ret = SQLBindCol(sql_stmt, 2, SQL_C_CHAR,
			     emb_info.ip_macchina, 50, &signal);

	sql_ret = SQLBindCol(sql_stmt, 3, SQL_C_FLOAT,
			     &emb_info.soglia_tensione,
			     sizeof(emb_info.soglia_tensione), &signal);

	sql_ret = SQLBindCol(sql_stmt, 4, SQL_C_FLOAT,
			     &emb_info.soglia_corrente,
			     sizeof(emb_info.soglia_corrente), &signal);

	sql_ret = SQLBindCol(sql_stmt, 5, SQL_C_FLOAT,
			     &emb_info.k1, sizeof(emb_info.k1), &signal);

	sql_ret = SQLBindCol(sql_stmt, 6, SQL_C_FLOAT,
			     &emb_info.k2, sizeof(emb_info.k2), &signal);

	sql_ret = SQLExecDirect(sql_stmt, query, SQL_NTS);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLExecDirect%d\n", sql_ret);
		extract_error("SQLExecDirect", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLExecDirect\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	sql_ret = SQLFetch(sql_stmt);

	if (sql_ret != SQL_NO_DATA) {

		printDebug("Found informations for this machine into DB... ");

		if ((sql_ret != SQL_SUCCESS)
		    && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
			printf("Error SQLFetch %d\n", sql_ret);
			extract_error("SQLFetch ", sql_stmt, SQL_HANDLE_STMT);
			//printFatal("Error during SQLFetch \n");
			SQLCloseCursor(sql_stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
			return;
		}

		SQLCloseCursor(sql_stmt);

		if ((sql_ret != SQL_SUCCESS)
		    && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
			printf("Error SQLAllocHandle %d\n", sql_ret);
			extract_error("SQLAllocHandle", sql_hdbc,
				      SQL_HANDLE_DBC);
			//printFatal("Error during SQLAllocHandle\n");
			SQLCloseCursor(sql_stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
			return;
		}

		snprintf(query, 500,
			 "SELECT num_isola, cod_isola, k1 FROM Isole WITH (NOLOCK)"
			 " WHERE num_impianto = %d", emb_info.num_impianto);

		sql_ret = SQLBindCol(sql_stmt, 1, SQL_C_SLONG,
				     &num_isola, sizeof(num_isola), &signal);

		sql_ret = SQLBindCol(sql_stmt, 2, SQL_C_SLONG,
				     &cod_isola, sizeof(cod_isola), &signal);

		sql_ret = SQLBindCol(sql_stmt, 3, SQL_C_FLOAT,
				     &k1, sizeof(k1), &signal);

		sql_ret = SQLExecDirect(sql_stmt, query, SQL_NTS);

		if ((sql_ret != SQL_SUCCESS)
		    && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
			printf("Error SQLExecDirect%d\n", sql_ret);
			extract_error("SQLExecDirect", sql_stmt,
				      SQL_HANDLE_STMT);
			//printFatal("Error during SQLExecDirect\n");
			SQLCloseCursor(sql_stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
			return;
		}

		i = 0;
		j = 0;
		// il primo plcfetch prende isole 1-4, il secondo 5-8
		while (SQL_SUCCEEDED(SQLFetch(sql_stmt))) {
			if (signal != SQL_NULL_DATA) {
				if (cfg.instancenum == 1 && i >= 0 && i <= 3) {
					emb_info.array_isole[j] = num_isola;
					emb_info.array_cod_isole[j] = cod_isola;
					emb_info.array_island_k1[j] = k1;
					j++;
				} else if (cfg.instancenum == 2 && i > 3
					   && i <= 7) {
					emb_info.array_isole[j] = num_isola;
					emb_info.array_cod_isole[j] = cod_isola;
					emb_info.array_island_k1[j] = k1;
					j++;
				}
			}
			i++;
		}

		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

		emb_info.num_isole = i;

		if (!i) {
			printError
			    ("Found no islands for this machine into db... i will"
			     "no write any informations on db...");
		}

		if (cfg.verbose) {
			int i;
			printf("ip_macchina: %s\n", emb_info.ip_macchina);
			printf("num_impianto (id): %d\n",
			       emb_info.num_impianto);
			printf("k1: %f, k2: %f\n", emb_info.k1, emb_info.k2);
			printf("soglia_tensione: %f, corrente: %f\n",
			       emb_info.soglia_tensione,
			       emb_info.soglia_corrente);
			printf("tot. isole: %d\n", emb_info.num_isole);
			for (i = 0; i < emb_info.num_isole; i++)
				printf
				    ("num_isola (id): %d, cod_isola: %d, island_k1: %f\n",
				     emb_info.array_isole[i],
				     emb_info.array_cod_isole[i],
				     emb_info.array_island_k1[i]);
		}

	} else {
		if (cfg.verbose) {
			printFatal
			    ("No database information found for this machine... "
			     "continuing without these informations, no data writed into db");
			SQLCloseCursor(sql_stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
			return;
		}
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
	}
}

void plc_insert(int machine, int island, int event, float volt, int curr,
		float segcc)
{

	SQLHSTMT sql_stmt;
	SQLRETURN sql_ret;
	SQLCHAR query[255];

	sql_ret = SQLAllocHandle(SQL_HANDLE_STMT, sql_hdbc, &sql_stmt);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLAllocHandle %d\n", sql_ret);
		extract_error("SQLAllocHandle", sql_hdbc, SQL_HANDLE_DBC);
		//printFatal("Error during SQLAllocHandle\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	snprintf(query, 255, "INSERT INTO PLC_Insert(num_macchina, "
		 "num_isola, cod_evento, val_volt, val_amp, seg_cc) "
		 "VALUES (%d, %d, %d, %.2f, %d, %.2f)", machine, island,
		 event, volt, curr, segcc);

	printf("%s\n", query);

	sql_ret = SQLExecDirect(sql_stmt, query, SQL_NTS);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLExecute%d\n", sql_ret);
		extract_error("SQLExecute ", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLExecute\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	SQLCloseCursor(sql_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

}

void plc_update(int machine, int island, float volt, int curr, float segcc)
{

	SQLCHAR query[255];
	SQLHSTMT sql_stmt;
	SQLRETURN sql_ret;
	SQLINTEGER rows;

	sql_ret = SQLAllocHandle(SQL_HANDLE_STMT, sql_hdbc, &sql_stmt);

	snprintf(query, 255, "UPDATE PLC_Update WITH (ROWLOCK) SET "
		 "val_volt = %.2f, val_amp = %d, seg_cc = %.2f "
		 "WHERE num_macchina = %d AND num_isola = %d", volt,
		 curr, segcc, machine, island);

	printf("%s\n", query);

	sql_ret = SQLExecDirect(sql_stmt, query, SQL_NTS);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLExecute%d\n", sql_ret);
		extract_error("SQLExecute", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLExecute\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	sql_ret = SQLRowCount(sql_stmt, &rows);
	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLExecute%d\n", sql_ret);
		extract_error("SQLExecute", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLExecute\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}
	SQLCloseCursor(sql_stmt);
	//SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

	if (rows == 0) {

		//SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

		snprintf(query, 255,
			 "INSERT INTO PLC_Update (num_macchina, num_isola, "
			 "val_volt, val_amp, seg_cc) VALUES (%d, %d, "
			 "%.2f, %d, %.2f)", machine, island, volt, curr, segcc);

		printf("%s\n", query);

		sql_ret = SQLExecDirect(sql_stmt, query, SQL_NTS);

		if ((sql_ret != SQL_SUCCESS)
		    && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
			printf("Error SQLExecute%d\n", sql_ret);
			extract_error("SQLExecute", sql_stmt, SQL_HANDLE_STMT);
			//printFatal("Error during SQLExecute\n");
			SQLCloseCursor(sql_stmt);
			SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
			return;
		}

		SQLCloseCursor(sql_stmt);
		//SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

	}

	SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

}

void get_db_voltagecurrent(int num_impianto, int num_isola, int *current,
			   float *voltage)
{

	SQLHSTMT sql_stmt;
	SQLRETURN sql_ret;
	float volt;
	int curr;

	sql_ret = SQLAllocHandle(SQL_HANDLE_STMT, sql_hdbc, &sql_stmt);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLAllocHandle %d\n", sql_ret);
		extract_error("SQLAllocHandle", sql_hdbc, SQL_HANDLE_DBC);
		//printFatal("Error during SQLAllocHandle\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	sql_ret =
	    SQLPrepare(sql_stmt,
		       "SELECT val_volt, val_amp FROM PLC_Update WITH (NOLOCK)"
		       " WHERE num_macchina=? AND num_isola=?", SQL_NTS);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLPrepare %d\n", sql_ret);
		extract_error("SQLPrepare ", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLPrepare\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	SQLBindParameter(sql_stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
			 SQL_INTEGER, 0, 0, &num_impianto,
			 sizeof(num_impianto), NULL);

	SQLBindParameter(sql_stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
			 SQL_INTEGER, 0, 0, &num_isola, sizeof(num_isola),
			 NULL);


	sql_ret =
	    SQLBindCol(sql_stmt, 1, SQL_C_FLOAT, &volt, sizeof(volt), NULL);

	sql_ret =
	    SQLBindCol(sql_stmt, 2, SQL_C_LONG, &curr, sizeof(curr), NULL);

	sql_ret = SQLExecute(sql_stmt);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLExecute%d\n", sql_ret);
		extract_error("SQLExecute ", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLExecute\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	sql_ret = SQLFetch(sql_stmt);

	if (sql_ret != SQL_NO_DATA) {
		*voltage = volt;
		*current = curr;
	} else {
		*voltage = 0;
		*current = 0;
	}

	SQLCloseCursor(sql_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

}

void get_db_encoder(int num_impianto, int num_isola, float *encoder)
{

	SQLHSTMT sql_stmt;
	SQLRETURN sql_ret;
	float enc;

	sql_ret = SQLAllocHandle(SQL_HANDLE_STMT, sql_hdbc, &sql_stmt);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLAllocHandle %d\n", sql_ret);
		extract_error("SQLAllocHandle", sql_hdbc, SQL_HANDLE_DBC);
		//printFatal("Error during SQLAllocHandle\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	sql_ret =
	    SQLPrepare(sql_stmt,
		       "SELECT seg_cc FROM PLC_Update WITH (NOLOCK)"
		       " WHERE num_macchina=? AND num_isola=?", SQL_NTS);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLPrepare %d\n", sql_ret);
		extract_error("SQLPrepare ", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLPrepare\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	SQLBindParameter(sql_stmt, 1, SQL_PARAM_INPUT, SQL_C_LONG,
			 SQL_INTEGER, 0, 0, &num_impianto,
			 sizeof(num_impianto), NULL);

	SQLBindParameter(sql_stmt, 2, SQL_PARAM_INPUT, SQL_C_LONG,
			 SQL_INTEGER, 0, 0, &num_isola, sizeof(num_isola),
			 NULL);

	sql_ret = SQLBindCol(sql_stmt, 1, SQL_C_FLOAT, &enc, sizeof(enc), NULL);

	sql_ret = SQLExecute(sql_stmt);

	if ((sql_ret != SQL_SUCCESS) && (sql_ret != SQL_SUCCESS_WITH_INFO)) {
		printf("Error SQLExecute%d\n", sql_ret);
		extract_error("SQLExecute ", sql_stmt, SQL_HANDLE_STMT);
		//printFatal("Error during SQLExecute\n");
		SQLCloseCursor(sql_stmt);
		SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);
		return;
	}

	sql_ret = SQLFetch(sql_stmt);

	if (sql_ret != SQL_NO_DATA) {
		*encoder = enc;
	} else {
		*encoder = 0;
	}

	SQLCloseCursor(sql_stmt);
	SQLFreeHandle(SQL_HANDLE_STMT, sql_stmt);

}

void do_sql_things(datarecord d)
{

	short int bit_main[3];
	short int bit_island[4][3];
	short int i, j;

	if (!curr_timestamp) {
		curr_timestamp = time(NULL);
	}
	// macchina on/off
	bit_main[0] = GETBITS(d.databytes[0], 0, 1);
	// rottura filo
	bit_main[1] = GETBITS(d.databytes[0], 1, 1);
	// caduta ballerino
	bit_main[2] = GETBITS(d.databytes[0], 2, 1);

	if (emb_info.num_isole >= 1) {
		// reset conteggio isola 1
		bit_island[0][0] = GETBITS(d.databytes[1], 0, 1);
		// fine conteggio isola 1
		bit_island[0][1] = GETBITS(d.databytes[1], 1, 1);
		// stampa isola 1
		bit_island[0][2] = GETBITS(d.databytes[1], 2, 1);
	}

	if (emb_info.num_isole >= 2) {
		// reset conteggio isola 2
		bit_island[1][0] = GETBITS(d.databytes[1], 4, 1);
		// fine conteggio isola 2
		bit_island[1][1] = GETBITS(d.databytes[1], 5, 1);
		// stampa isola 2
		bit_island[1][2] = GETBITS(d.databytes[1], 6, 1);
	}

	if (emb_info.num_isole >= 3) {
		// reset conteggio isola 3
		bit_island[2][0] = GETBITS(d.databytes[2], 0, 1);
		// fine conteggio isola 3
		bit_island[2][1] = GETBITS(d.databytes[2], 1, 1);
		// stampa isola 3
		bit_island[2][2] = GETBITS(d.databytes[2], 2, 1);
	}

	if (emb_info.num_isole >= 4) {
		// reset conteggio isola 4
		bit_island[3][0] = GETBITS(d.databytes[2], 4, 1);
		// fine conteggio isola 4
		bit_island[3][1] = GETBITS(d.databytes[2], 5, 1);
		// stampa isola 4
		bit_island[3][2] = GETBITS(d.databytes[2], 6, 1);
	}
	// TODO: make a function
	if (bit_main[0]) {
		// MARCIA
		last_arresto = 0;
		if (first_loop && cfg.nosendstart) {
			last_marcia = MARCIA;
		}
		if (last_marcia != MARCIA) {
			for (i = 0; i < emb_info.num_isole; i++) {
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i], MARCIA,
					   d.voltage, d.current, d.encoder[i]);
			}
			last_marcia = MARCIA;
		}
	} else if (!bit_main[0]) {
		// ARRESTO
		last_marcia = 0;
		if (first_loop && cfg.nosendstart) {
			last_arresto = ARRESTO;
		}
		if (last_arresto != ARRESTO) {
			for (i = 0; i < emb_info.num_isole; i++) {
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i],
					   ARRESTO, d.voltage, d.current,
					   d.encoder[i]);
			}
			last_arresto = ARRESTO;
		}
	}
	// TODO: make a function
	if (bit_main[1]) {
		// ROTTURA_FILO
		if (first_loop && cfg.nosendstart) {
			last_rotturafilo = ROTTURA_FILO;
		}
		if (last_rotturafilo != ROTTURA_FILO) {
			for (i = 0; i < emb_info.num_isole; i++) {
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i],
					   ROTTURA_FILO, d.voltage,
					   d.current, d.encoder[i]);
			}
			last_rotturafilo = ROTTURA_FILO;
		}
	} else
		last_rotturafilo = 0;

	// TODO: make a function
	if (bit_main[2]) {
		// ROTTURA_BALLERINO
		if (first_loop && cfg.nosendstart) {
			last_rotturaballerino = ROTTURA_BALLERINO;
		}
		if (last_rotturaballerino != ROTTURA_BALLERINO) {
			for (i = 0; i < emb_info.num_isole; i++) {
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i],
					   ROTTURA_BALLERINO, d.voltage,
					   d.current, d.encoder[i]);
			}
			last_rotturaballerino = ROTTURA_BALLERINO;
		}
	} else
		last_rotturaballerino = 0;

	if (first_loop && cfg.nosendstart == 0) {
		int t;
		for (t = 0; t < emb_info.num_isole; t++) {
			plc_insert(emb_info.num_impianto,
				   emb_info.array_isole[t], START_PLCFETCH,
				   d.voltage, d.current, d.encoder[t]);
		}
	}

	for (i = 0; i < emb_info.num_isole; i++) {

		// TODO: make a function
		if (bit_island[i][0]) {
			// RESET_CONTACOLPI_ISOLA
			if (first_loop && cfg.nosendstart) {
				last_resetcontacolpi[i] =
				    RESET_CONTACOLPI_ISOLA;
			}
			if (last_resetcontacolpi[i] != RESET_CONTACOLPI_ISOLA) {
				float last_encoder = 0;
				get_db_encoder(emb_info.num_impianto,
					       emb_info.array_isole[i],
					       &last_encoder);
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i],
					   RESET_CONTACOLPI_ISOLA,
					   d.voltage, d.current, last_encoder);
				last_resetcontacolpi[i] =
				    RESET_CONTACOLPI_ISOLA;
			}
		} else
			last_resetcontacolpi[i] = 0;

		// TODO: make a function
		if (bit_island[i][1]) {
			// FINE_CONTEGGIO_ISOLA
			if (first_loop && cfg.nosendstart) {
				last_fineconteggio[i] = FINE_CONTEGGIO_ISOLA;
			}
			if (last_fineconteggio[i] != FINE_CONTEGGIO_ISOLA) {
				float last_encoder = 0;
				get_db_encoder(emb_info.num_impianto,
					       emb_info.array_isole[i],
					       &last_encoder);
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i],
					   FINE_CONTEGGIO_ISOLA, d.voltage,
					   d.current, last_encoder);
				last_fineconteggio[i] = FINE_CONTEGGIO_ISOLA;
			}
		} else
			last_fineconteggio[i] = 0;

		// TODO: make a function
		if (bit_island[i][2]) {
			// STAMPA_ISOLA
			if (first_loop && cfg.nosendstart) {
				last_stampaisola[i] = STAMPA_ISOLA;
			}
			if (last_stampaisola[i] != STAMPA_ISOLA) {
				plc_insert(emb_info.num_impianto,
					   emb_info.array_isole[i],
					   STAMPA_ISOLA, d.voltage,
					   d.current, d.encoder[i]);
				last_stampaisola[i] = STAMPA_ISOLA;
			}
		} else
			last_stampaisola[i] = 0;

	}

	// ACCROCCHIO...
	for (i = 0; i < emb_info.num_isole; i++) {
		get_db_voltagecurrent(emb_info.num_impianto,
				      emb_info.array_isole[i], &db_current,
				      &db_voltage);
	}

	// TODO: make a function
	if ((d.voltage <= (db_voltage - emb_info.soglia_tensione)) ||
	    (d.voltage >= (db_voltage + emb_info.soglia_tensione))) {

		// SOGLIA_TENSIONE
		for (j = 0; j < emb_info.num_isole; j++) {
			plc_insert(emb_info.num_impianto,
				   emb_info.array_isole[j],
				   SOGLIA_TENSIONE, d.voltage, d.current,
				   d.encoder[j]);
		}
	}
	// TODO: make a function
	if ((d.current <= (db_current - (int) emb_info.soglia_corrente)) ||
	    (d.current >= (db_current + (int) emb_info.soglia_corrente))) {

		// SOGLIA_CORRENTE
		for (j = 0; j < emb_info.num_isole; j++) {
			plc_insert(emb_info.num_impianto,
				   emb_info.array_isole[j],
				   SOGLIA_CORRENTE, d.voltage, d.current,
				   d.encoder[j]);
		}
	}

	if (time(NULL) - curr_timestamp >= cfg.update_secs) {
		for (i = 0; i < emb_info.num_isole; i++) {
			plc_update(emb_info.num_impianto,
				   emb_info.array_isole[i], d.voltage,
				   d.current, d.encoder[i]);
		}
		curr_timestamp = time(NULL);
	}

	first_loop = 0;

}
