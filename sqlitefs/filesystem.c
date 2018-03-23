/** @file */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <fuse.h>
#include <stdio.h>
#include <stdbool.h>
#include "database.h"
#include <stdlib.h>
#include <unistd.h>

#define DATABASE "test.db"

static const int num_tables = 3;
static char *table_names[num_tables];
static char *table_contents[num_tables];

static int
sqlitefs_getattr(const char *path, struct stat *string_buf)
{
	memset(string_buf, 0, sizeof(struct stat));   
	if (strcmp(path, "/") == 0) {
		string_buf->st_mode = S_IFDIR | 0755;
		string_buf->st_nlink = 2 + num_tables;
	} else {
		for (int i = 0; i < num_tables; i++) {
			if (strcmp(path + 1, table_names[i]) == 0) {  /* drop leading '/'    */
				string_buf->st_mode = S_IFREG | 0444; /* RO for everyone     */
				string_buf->st_nlink = 1;             /* num of links (self) */
				string_buf->st_size = strlen(table_contents[i]);
			}
		}
		// return -ENOENT; /* TODO: what here? */
	}
	return 0;
}

static int
sqlitefs_open(const char *path, struct fuse_file_info *fi)
{
	for (int i = 0; i < num_tables; i++) {
		if (strcmp(path+1, table_names[i]) == 0               /* drop leading '/' */
		    /* && (fi->flags & O_ACCMODE) != O_RDONLY */) {
			return 0;
		}
	}
	return -ENOENT;                                               /* file doesn't exist */
}

static int
sqlitefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	if (strcmp(path, "/") != 0) /* We only recognize the root directory. */
		return -ENOENT;
    
	filler(buf, ".", NULL, 0);           /* Current directory (.)  */
	filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
    
	for (int i = 0; i < num_tables; i++) {
		filler(buf, table_names[i], NULL, 0);
	}
	return 0;
}

static int
sqlitefs_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	for (int i = 0; i < num_tables; i++) {             /* check all tables */
		if (strcmp(path+1, table_names[i]) == 0) { /* found a match */
			size_t len = strlen(table_contents[i]);
			if(offset >= len) {                          /* reading past the end of the file */
				return 0;
			}
			if(offset + size > len) {                    /* reading some of the file */
				memcpy(buf, table_contents[i] + offset, len - offset);
				return len - offset;
			}
			memcpy(buf, table_contents[i] + offset, size);    /* read whole thing */
			return size;
		}
	}
	return -ENOENT;
}

static struct fuse_operations sqlitefs_filesystem_operations = {
	.getattr = sqlitefs_getattr,    /* To provide size, permissions, etc. */
	.open    = sqlitefs_open,       /* To enforce read-only access.       */
	.read    = sqlitefs_read,       /* To provide file content.           */
	.readdir = sqlitefs_readdir     /* To provide directory listing.      */
};

int
main(int argc, char **argv)
{
	int is_accessible = access(argv[1], 4);
	if (is_accessible == 0) {
		// Get the names of the tables
		get_table_names(DATABASE, table_names, MAX_NUMBER_OF_TABLES);

		// Load the contents of the tables
		char *contents;
		for (int i = 0; i < sizeof(table_names)/sizeof(table_names[0]); i++) {
			contents = NULL;
			get_table_content(DATABASE, table_names[i], &contents);
			table_contents[i] = contents;
		}
		return fuse_main(argc, argv, &sqlitefs_filesystem_operations, NULL);
	} else {
		fprintf(stderr, "ERROR: %s\n", strerror(errno));
		return 1;
	}
}
