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

static int
sqlitefs_getattr(const char *path, struct stat *string_buf)
{
    printf("CALLED sqlitefs_getattr\n");
    memset(string_buf, 0, sizeof(struct stat));
    
    if (strcmp(path, "/") == 0)
    {
        string_buf->st_mode = S_IFDIR | 0755;
        string_buf->st_nlink = 2 + num_tables;
    }
    else
    {
        for (int i = 0; i < num_tables; i++)
        {
            if (strcmp(path + 1, table_names[i]) == 0) /* drop leading /? */
            {
                string_buf->st_mode = S_IFREG | 0444;
                string_buf->st_nlink = 1;
                string_buf->st_size = 0;
            }
        }
        // return -ENOENT;
    }
    return 0;
}

static int
sqlitefs_open(const char *path, struct fuse_file_info *fi)
{
    for (int i = 0; i < num_tables; i++)
    {
        if (strcmp(path+1, table_names[i]) == 0            // drop leading '/'
            // && (fi->flags & O_ACCMODE) != O_RDONLY
            )
        {
            return 0;                                      // success
            // return -EACCES;                                // permission denied
        }
    }
    return -ENOENT;                                        // file doesn't exist
}

static int
sqlitefs_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
              off_t offset, struct fuse_file_info *fi)
{
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
sqlitefs_read(const char *path, char *buf, size_t size, off_t offset,
            struct fuse_file_info *fi)
{
    // bool unknown_file = true;
    // for (int i = 0; i < num_tables; i++)
    // {
    //     if (strcmp(path+1, table_names[i]) == 0)
    //         unknown_file = false;
    // }
    // if (unknown_file)
    //     return -ENOENT;
    
    // char *content = malloc(1);
    // get_table_content(DATABASE, path, &content);
    // memcpy(buf, content, 0); /* Provide the content. */
    // fi->flags = O_RDONLY;
    // return (int)strlen(content);
    char *str = "Hello world!";
    *buf = *str; 
    return 13;
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
    // if (argc != 3)
    // {
    // 	fprintf(stderr, "USAGE:\n\t%s DBFILE MOUNT_POINT\n", argv[0]);
    // 	exit(1);
    // }

    int is_accessible = access(argv[1], 4);
    printf("Accessible? %d\n", is_accessible);
    if (is_accessible == 0)
    {
        fprintf(stderr, "FILESYSTEM MOUNTED\n");
        get_table_names(DATABASE, table_names, MAX_NUMBER_OF_TABLES);

        char *contents = NULL;
        for (int i = 0; i < sizeof(table_names)/sizeof(table_names[0]); i++)
        {
            printf("=======\n%s\n=======\n", table_names[i]);
            get_table_content(DATABASE, table_names[i], &contents);
            printf("%s\n\n", contents);
            contents = NULL;
        }
        return fuse_main(argc, argv, &sqlitefs_filesystem_operations, NULL);
        //return 0;
    }
    else
    {
        fprintf(stderr, "ERROR: %s\n", strerror(errno));
        return 1;
    }
}
