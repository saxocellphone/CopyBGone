#ifndef GUARD_DATABASE_H
#define GUARD_DATABASE_H

#include <stdlib.h>

#include "types.h"
//Hash type for k-grams
typedef unsigned int hash_t;
//Hash type for buckets in the table
typedef unsigned int hash_hash_t;
//The text position of a particular hash
typedef long position_t;
//the location (including the source file) of a k-gram hash
typedef struct _location_t {
    position_t pos;
    char * source_file;
} location_t;

//Storing the list of locations within the node
typedef struct _location_node_t {
    location_t   location;
    struct _location_node_t* next;
} location_node_t;

//Linked list for the location nodes
typedef struct _location_list_t {
    int*             size;
    location_node_t* head;
    location_node_t* tail;
} location_list_t;

//Linked list for node in case of collison
typedef struct _node_t {
    hash_t           hash;
    location_list_t* locs;
    struct _node_t*  next;
} node_t;

//Type of the fingerprint table
typedef struct _table_t {
    node_t**      buckets;
    int*          size;
} table_t;

void fingerprints_create(table_t* table, int size);

int fingerprints_add(table_t* table, unsigned int hash, location_t location);

int fingerprints_get(table_t* table, unsigned int hash, location_list_t* locations);

unsigned int hash_it(unsigned int x, int size);

#endif