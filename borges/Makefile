LDFLAGS = -losxfuse -lsqlite3
HEADERS = \
	-I/usr/local/include/osxfuse/fuse \
	-I/usr/local/Cellar/sqlite/3.21.0/include
DYNLIBS = \
	-L/usr/local/lib \
	-L/usr/local/Cellar/sqlite/3.21.0/lib
ENFLAGS = \
	-D_FILE_OFFSET_BITS=64 \
	-D_DARWIN_USE_64_BIT_INODE \
	-DFUSE_USE_VERSION=26

all: database.c filesystem.c
	gcc filesystem.c database.c -o borges ${LDFLAGS} ${HEADERS} ${DYNLIBS} ${ENFLAGS}

debug: database.c filesystem.c
	gcc filesystem.c database.c -g -o borges ${LDFLAGS} ${HEADERS} ${DYNLIBS} ${ENFLAGS}

clean:
	rm borges
