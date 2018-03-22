# SQLiteFS

> You who read me, are You sure of understanding my language?  
> -- Jorge Luis sqlitefs "The Library of Babel"

## About

SQLiteFS interprets a SQLite database as a directory full of CSV files.  Each
table is converted into its own file with the contents of the file being the
rows of the table. At this time the conversion is read-only, but that could
change in the future.

## Usage / Installation

SQLiteFS depends on two libraries in order to do its task:

- SQLite3 (libsqlite3)
- FUSE (libfuse)

SQLite3 should be fairly self-explanatory. FUSE allows one to write Filesystems
in USErspace. When navigating the mounted filesystem, SQLiteFS is reading the
database and converting that into files and their contents.

Building should be relatively straightforward as it is all ANSI C.
