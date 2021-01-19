#ifndef _STORAGE_
#define _STORAGE_

#include <glib.h>

typedef struct database {
    
    GHashTable *storage;
    char* path;
    char* last_modification;

} *DB;

void load_contact_database(DB storage, char* path);

void print_contact_database(DB storage);

int has_contact(DB db, char* key);

char* get_contact(DB db, char* key);

int has_storage_been_modified(DB db);

void update_storage_database(DB db);

#endif
