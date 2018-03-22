/** @file */

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "database.h"

static bool
open_db(const char *dbname, sqlite3 **db)
{
	int rc = sqlite3_open(dbname, db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(*db));
		sqlite3_close(*db);
		return false;
	}
	return true;
}

/**
 * Called once per result row. We are searching for the names of the tables in
 * this database.
 */
static int
get_tables_callback(void *_tblnames, int argc, char **argv, char **azColName)
{
	static int name_index = 0;      // maintains name_index between invocations
	char **table_names = _tblnames; // cast to char**
	
	for(int i=0; i<argc; i++)
	{
		if (strncmp(azColName[i], "name", 4) == 0
			&& strncmp(argv[i], "sqlite_sequence", 15) != 0)
		{
			size_t len = strnlen(argv[i], MAX_TABLE_NAME) + 1;
			table_names[name_index] = (char *)malloc(len);
			strncpy(table_names[name_index], argv[i], len);
			name_index++;
		}
	}
	return 0;
}

/**
 * Fill the provided array of strings with names of the tables in the database
 *
 * @param dbname path of the database to inspect
 * @param table_names an array of pointers to char for storing result
 * @param max_tables the number of tables expected
 */
bool
get_table_names(char *dbname, char *table_names[], size_t max_tables)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	
	if (!open_db(dbname, &db)) return false;
	
	int rc = sqlite3_exec(db, "SELECT * FROM sqlite_master WHERE type = 'table'",
						  get_tables_callback, table_names, &zErrMsg);
	if( rc!=SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return false;
	}
	sqlite3_close(db);
	return true;
}

static int
read_table_callback(char **contents, int argc, char **argv, char **azColName)
{
	char *start;
	char *end;
	
	// print the header row before any of the content
	if (!(*contents)) /* contents is null */
	{
		int i;
		int header_size = 2 + (argc - 1);
		for (i = 0; i < argc; i++)
			header_size = header_size + sizeof(azColName[i]);
		end = malloc(1 + header_size);
		start = end;
		for (i = 0; i < argc; i++)
		{
			end = stpcpy(end, azColName[i]);
			if (i < argc - 1)
				end = stpcpy(end, ",");
		}
		*contents = start;
	}

	// measure the size of this row, plus the eventual comma separators
	// foo,bar,baz\n\0
	// = (2 + 2) + 3 + 3 + 3
	// = (argc - 1) + 2 + sum(sizeof(argv[i]))
	int row_size = 2 + (argc - 1);
	for (int i=0; i < argc; i++)
		row_size = row_size + sizeof(argv[i]);
	
	// allocate new total size to accommodate incoming row & copy it in.
	end = malloc(strlen(*contents) + 1 + row_size);
	start = end;
	
	end = stpcpy(end, *contents);
	end = stpcpy(end, "\n");
	
	for (int i=0; i<argc; i++)
	{
		end = stpcpy(end, argv[i]);
		if (i < argc - 1)
			end = stpcpy(end, ",");
	}
	*contents = start;
	return 0;
}

/**
 * Read the contents of table.
 */
bool
get_table_content(const char *dbname, const char *table_name, char **result)
{
	sqlite3 *db;
	char *errMsg;
	
	if (!open_db(dbname, &db)) return false;
	
	char fmt[] = "SELECT * FROM %s";
	char sql_statement[MAX_TABLE_NAME + sizeof(fmt)];
	snprintf(sql_statement, sizeof(sql_statement), fmt, table_name);
	
	int rc =
		sqlite3_exec(db, sql_statement, read_table_callback, result, &errMsg);
	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", errMsg);
		sqlite3_free(errMsg);
		return false;
	}
	sqlite3_close(db);
	return true;
}
