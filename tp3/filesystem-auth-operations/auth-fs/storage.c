
#define _GNU_SOURCE

#include "include/storage.h"

#include <string.h>
#include <stdio.h>
#include <glib.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

void load_contact_database(DB db, char* path) {

    db -> path = strdup(path);
    
    struct stat attrib;
    lstat(db -> path, &attrib);
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));

    db -> last_modification = strdup(time); 

    db -> storage = g_hash_table_new(g_str_hash, g_str_equal);

    FILE *fp;
    fp = fopen(db->path, "r");
    
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char delim[] = ":";

    if (fp) {

        while((read = getline(&line, &len, fp)) != -1) {
            
            if (read <= 1) continue;

            //<user>::<contact>
            char *ptr_key = strtok(line, delim);
            char *ptr_val = strtok(NULL, delim);
            
            ptr_key[strcspn(ptr_key, "\n")]=0; 
            ptr_val[strcspn(ptr_val, "\n")]=0; 
            
            g_hash_table_insert(db->storage, g_strdup(ptr_key), g_strdup(ptr_val));
        }
    }
}

void print_contact_database(DB db) {
    
    printf("---------------------------------------------\n");

    if (db && db -> storage) {
   
        //display all database

        int iteration = 0;

        GList* keys = g_hash_table_get_keys(db->storage);

        for (GList* iter = keys; iter; iter = iter -> next) {
            
            char* key = (char*) iter -> data;
            char* val = (char*) g_hash_table_lookup(db->storage, key);

            printf("[#%d] key: %s => %s\n", iteration, key, val);
            
            iteration++;
        }

        //get last time modified

        printf("last time modified: %s\n", db -> last_modification);

    } else {

        printf("Contact dabase = NULL (not defined)");
    }
    
    printf("---------------------------------------------\n");
}

int has_contact(DB db, char* key) {
    
    return g_hash_table_lookup(db->storage, key) != NULL;
}   

char* get_contact(DB db, char* key) {
    
    return g_hash_table_lookup(db->storage, key);
}

int has_storage_been_modified(DB db) {
    
    struct stat attrib;
    int r = lstat(db -> path, &attrib);
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));
   
    return strcmp(time, db->last_modification);
}

void update_storage_database(DB db) {

    FILE *fp;
    fp = fopen(db->path, "r");
    
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    char delim[] = ":";

    if (fp) {

        while((read = getline(&line, &len, fp)) != -1) {
            
            if (read <= 1) continue;

            //<user>::<contact>
            char *ptr_key = strtok(line, delim);
            char *ptr_val = strtok(NULL, delim);
            
            ptr_key[strcspn(ptr_key, "\n")]=0; 
            ptr_val[strcspn(ptr_val, "\n")]=0; 
            
            g_hash_table_insert(db->storage, g_strdup(ptr_key), g_strdup(ptr_val));
        }
    }

    struct stat attrib;
    lstat(db -> path, &attrib);
    char time[50];
    strftime(time, 50, "%Y-%m-%d %H:%M:%S", localtime(&attrib.st_mtime));
    
    free(db->last_modification);
    db->last_modification=strdup(time);
}
