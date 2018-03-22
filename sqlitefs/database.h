//
//  database.h
//  sqlitefs/sqlitefs
//
//  Created by Chris Wilson on 2/8/18.
//  Copyright © 2018 Chris Wilson. All rights reserved.
//

#include <stdlib.h>

#ifndef database_h
/** For simplicity we cap the number of tables here */
#define MAX_NUMBER_OF_TABLES 50

/**
 * The max number of characters in a table name - nobody wants to look at a
 * super long filename.
 */
#define MAX_TABLE_NAME 50
bool get_table_names(char*, char* [], size_t);
bool get_table_content(const char *, const char *, char **);

/** The "is file readable" access mode */
#define READABLE 4

#define database_h
#endif /* database_h */
