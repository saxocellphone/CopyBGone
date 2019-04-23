#include "fingerprints.h"
#include "kgram.h"
#include <stdio.h>

#define ALPHA 26 //Number of characters in alphabet
/**
 * Karp-Rabin hash generator. Generates hash from a given k-gram.
 * */

//hash generator type
typedef struct _hash_gen_t {
    int kgram_len;
    long prime;
    int h;
    char* curr_kgram;
    hash_t curr_hash;
} hash_gen_t;

hash_t create_hash_generator(hash_gen_t** generator, int k, long prime, char* first_kgram);

hash_t generate_next_hash(hash_gen_t* generator, char* new_kgram);