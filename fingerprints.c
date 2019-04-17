#include "fingerprints.h"

void fingerprints_create(table_t* table) {
    table->buckets = (node_t*) malloc(SIZE * sizeof(struct _node_t));
}
/**
 * Gets a linked list of all the locations of a fingerprint
 * 
 * Returns 0 if fingerprint isn't found, returns 1 if it is found.
 * */
int fingerprints_get(table_t* table, unsigned int hash, location_list_t* locations_list) {
    unsigned int bucket_id = hash_it(hash);
    node_t* curr = table->buckets[bucket_id];
    while(curr){
        if(hash == curr->hash){
            locations_list = curr->locs;
            return 1;
        }
    }
    return 0;
}

/**
 * Adds a fingerprint to the list of known fingerprints.
 * 
 * Returns 0 if it is a new fingerprint, returns 1 if it is an existing fingerprint.
 * */
int fingerprints_add(table_t* table, unsigned int hash) {
    unsigned int bucket_id = hash_it(hash);
    location_list_t* locations_list;
    if(fingerprints_get(table, hash, locations_list) == 1){
        location_node_t* new_location = (location_node_t*) malloc(sizeof(location_node_t));
        if(locations_list->size == 0){
            locations_list->head = locations_list->ptr = locations_list->tail = new_location;
        } else {
            locations_list->tail->next = new_location;
            locations_list->tail = new_location;
        }
        locations_list->size++;
        return 1;
    } else {
        node_t* new_node = (node_t*) malloc(sizeof(struct _node_t));
        new_node->hash = hash;
        locations_list = (location_list_t*) malloc(sizeof(location_list_t));
        locations_list->size = (int*) calloc(1, sizeof(int));
        new_node->locs = (location_t*) malloc(sizeof(struct _location_t));
        node_t* curr = table->buckets[bucket_id];
        while(curr){
            //Doing loop for node because chance of collision is relatively small
            curr = curr->next;
        }
        curr->next = new_node;
        return 0;
    }
}

unsigned int hash_it(unsigned int x) {
    //https://stackoverflow.com/questions/664014/what-integer-hash-function-are-good-that-accepts-an-integer-hash-key/12996028
    //This is apparently good for 32-bit numbers, moding it by SIZE to ensure valid index
    x = ((x >> 16) ^ x) * 0x45d9f3b % SIZE;
    x = ((x >> 16) ^ x) * 0x45d9f3b % SIZE;
    x = (x >> 16) ^ x;
    return x;
}