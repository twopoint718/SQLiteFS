#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>

#define MAX_NUMBER_OF_TABLES 50
#define MAX_TABLE_NAME 50

char *table_names[MAX_NUMBER_OF_TABLES];
int name_index = 0;

static int get_tables_callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	int i;
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

int main(int argc, char **argv)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	if( argc!=3 )
	{
		fprintf(stderr, "Usage: %s DATABASE SQL-STATEMENT\n", argv[0]);
		return(1);
	}
	rc = sqlite3_open(argv[1], &db);
	if( rc )
	{
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(1);
	}
	rc = sqlite3_exec(db, argv[2], get_tables_callback, 0, &zErrMsg);
	if( rc!=SQLITE_OK )
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	int j = 0;
	while(table_names[j] != NULL) {
		printf("table: %s\n", table_names[j++]);
	}
	sqlite3_close(db);
	return 0;
}
