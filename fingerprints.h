#ifndef GUARD_DATABASE_H
#define GUARD_DATABASE_H

#include <stdlib.h>

#include "types.h"
#define SIZE 5000

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

typedef struct _table_t {
    node_t**      buckets;
    int*          size;
} table_t;

void fingerprints_create(table_t* table, int size);

int fingerprints_add(table_t* table, unsigned int hash, location_t location);

int fingerprints_get(table_t* table, unsigned int hash, location_list_t* locations);

unsigned int hash_it(unsigned int x, int size);

#endif