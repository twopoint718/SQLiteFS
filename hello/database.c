/** @file */

#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/** For simplicity we cap the number of tables here */
#define MAX_NUMBER_OF_TABLES 50

/**
 * The max number of characters in a table name - nobody wants to look at a
 * super long filename.
 */
#define MAX_TABLE_NAME 50

static bool
open_db(char *dbname, sqlite3 **db)
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
read_table_callback(void *_contents, int argc, char **argv, char **azColName)
{
	// measure the size of this row, plus the eventual comma separators
	// foo,bar,baz\n\0
	// = (2 + 2) + 3 + 3 + 3
	// = (argc - 1) + 2 + sum(sizeof(argv[i]))
	int row_size = 2 + (argc - 1);              // newline, null, separators
	for (int i=0; i < argc; i++)
		row_size = row_size + sizeof(argv[i]);
	
	// allocate new total size to accomodate incoming row & copy it in.
	char *contents = realloc(_contents, strlen(contents) + row_size);
	char *end = contents;
	for (int i=0; i<argc; i++)
	{
		end = stpcpy(end, argv[i]);
	}
	return 0;
}

/**
 * Read the contents of table.
 */
bool
get_table_content(char *dbname, const char *table_name, char **result)
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
