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

/** An array of pointers to the table names */
//char *table_names[MAX_NUMBER_OF_TABLES];

/**
 * Called once per result row. We are searching for the names of the tables in
 * this database.
 */
static int
get_tables_callback(char *table_names[], int argc, char **argv, char **azColName)
{
	int i;
	static int name_index = 0; /// maintains name_index between invocations
	size_t len;
	for(i=0; i<argc; i++)
	{
		if (strncmp(azColName[i], "name", 4) == 0
			&& strncmp(argv[i], "sqlite_sequence", 15) != 0)
		{
			len = strnlen(argv[i], MAX_TABLE_NAME) + 1;
			table_names[name_index] = (char *) malloc(len);
			strncpy(table_names[name_index], argv[i], len);
			name_index++;
		}
	}
	return 0;
}

/** Fill the provided array of strings with names of the tables in database
 * dbname
 * @param dbname name of the database to inspect
 * @param table_names an array of pointers to char for storing result
 * @param max_tables the number of tables expected
 */
bool
get_table_names(char* dbname, char* table_names[], size_t max_tables)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	rc = sqlite3_open(dbname, &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return false;
	}
	rc = sqlite3_exec(db, "SELECT * FROM sqlite_master WHERE type = 'table'",
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


/**
 * Open the given database file. Read the database file's contents and extract
 * all user-created tables. For each table, render its contents as a CSV file
 * in the FUSE filesystem (read-only at the moment).
 */
//int
//main(int argc, char **argv)
//{
//	const int num_tables = 3;
//	char *names[num_tables];
//	get_table_names("/Users/chris/test.db", names, num_tables);
//	for (int i = 0; i < 3; i++)
//	{
//		printf("table: %s\n", names[i]);
//	}
//	return 0;
//}

