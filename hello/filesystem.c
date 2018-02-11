/** @file */

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <fuse.h>
#include <stdio.h>
#include <stdbool.h>
#include "database.h"
#include <stdlib.h>

static const int num_tables = 3;
static char *table_names[num_tables];

static int
hello_getattr(const char *path, struct stat *stbuf)
{
	printf("CALLED hello_getattr\n");
	memset(stbuf, 0, sizeof(struct stat));
	
	if (strcmp(path, "/") == 0)
	{
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2 + num_tables;
	}
	else
	{
		for (int i = 0; i < num_tables; i++)
		{
			if (strcmp(path + 1, table_names[i]) == 0) /* drop leading /? */
			{
				stbuf->st_mode = S_IFREG | 0444;
				stbuf->st_nlink = 1;
				stbuf->st_size = 0;
			}
		}
		// return -ENOENT;
	}
	return 0;
}

static int
hello_open(const char *path, struct fuse_file_info *fi)
{
	for (int i = 0; i < num_tables; i++)
	{
		if (strcmp(path, table_names[i]) == 0
			&& (fi->flags & O_ACCMODE) != O_RDONLY)
		{
			return -EACCES;
		}
	}
	return -ENOENT;
}

static int
hello_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			  off_t offset, struct fuse_file_info *fi)
{
	printf("CALLED hello_readdir\n");
	if (strcmp(path, "/") != 0) /* We only recognize the root directory. */
		return -ENOENT;
	
	filler(buf, ".", NULL, 0);           /* Current directory (.)  */
	filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
	
	for (int i = 0; i < num_tables; i++)
	{
		// printf("saw: %s\n", table_names[i]);
		filler(buf, table_names[i], NULL, 0);
	}
	return 0;
}

static int
hello_read(const char *path, char *buf, size_t size, off_t offset,
		   struct fuse_file_info *fi)
{
	bool unknown_file = true;
	for (int i = 0; i < num_tables; i++)
	{
		if (strcmp(path, table_names[i]) == 0)
			unknown_file = false;
	}
	if (unknown_file)
		return -ENOENT;
	
	char *content = malloc(1);
	get_table_content("/Users/chris/test.db", path, &content);
	memcpy(buf, content, 0); /* Provide the content. */
	fi->flags = O_RDONLY;
	return (int)strlen(content);
}

static struct fuse_operations hello_filesystem_operations = {
	.getattr = hello_getattr, /* To provide size, permissions, etc. */
	.open    = hello_open,    /* To enforce read-only access.       */
	.read    = hello_read,    /* To provide file content.           */
	.readdir = hello_readdir, /* To provide directory listing.      */
};

int
main(int argc, char **argv)
{
	char *contents = malloc(50);
	get_table_content("/Users/chris/test.db", "artists", &contents);
	printf("%s", contents);
//	if (get_table_names("/Users/chris/test.db", table_names, num_tables))
//	{
//		printf("FILESYSTEM MOUNTED\n");
//		return fuse_main(argc, argv, &hello_filesystem_operations, NULL);
//	}
//	else
//	{
//		printf("ERROR: while listing table names\n");
//		return 1;
//	}
}
