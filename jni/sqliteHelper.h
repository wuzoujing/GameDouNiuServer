#include <stdio.h>
#include <string.h>

#ifndef USE_IN_ANDROID
#include <sqlite3.h>
#include "DouniuStruct.h"
  
#ifndef _Included_SQLITE_HELPER_H
#define _Included_SQLITE_HELPER_H

extern int select_callback(void *arg, int nr, char **values, char **names);
extern int select_by_callback(sqlite3 *db, const char* sql);
extern int select_by_table(sqlite3 *db, const char* sql);

extern int select_by_table_with_name(sqlite3 *db, const char* sql, DbUserInfo *userInfo);

#endif	//_Included_SQLITE_HELPER_H
#endif
