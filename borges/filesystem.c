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

static const int num_tables = 3;
static char *table_names[num_tables];

static int
borges_getattr(const char *path, struct stat *stbuf)
{
	printf("CALLED borges_getattr\n");
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
borges_open(const char *path, struct fuse_file_info *fi)
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
borges_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			  off_t offset, struct fuse_file_info *fi)
{
	printf("CALLED borges_readdir\n");
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
borges_read(const char *path, char *buf, size_t size, off_t offset,
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

static struct fuse_operations borges_filesystem_operations = {
	.getattr = borges_getattr, /* To provide size, permissions, etc. */
	.open    = borges_open,    /* To enforce read-only access.       */
	.read    = borges_read,    /* To provide file content.           */
	.readdir = borges_readdir /* To provide directory listing.      */
};

int
main(int argc, char **argv)
{
	if (argc != 3)
	{
		fprintf(stderr, "USAGE:\n\t%s DBFILE MOUNT_POINT\n", argv[0]);
		exit(1);
	}

	int is_accessible = access(argv[1], 4);
	if (is_accessible == 0)
	{
		fprintf(stderr, "FILESYSTEM MOUNTED\n");
		get_table_names(argv[1], table_names, MAX_NUMBER_OF_TABLES);

		char *contents = NULL;
		for (int i = 0; i < sizeof(table_names)/sizeof(table_names[0]); i++)
		{
			printf("=======\n%s\n=======\n", table_names[i]);
			get_table_content(argv[1], table_names[i], &contents);
			printf("%s\n\n", contents);
			contents = NULL;
		}
		//return fuse_main(argc, argv, &borges_filesystem_operations, NULL);
		return 0;
	}
	else
	{
		fprintf(stderr, "ERROR: database not accessible\n");
		return 1;
	}
}
