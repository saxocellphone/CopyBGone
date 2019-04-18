#include "fingerprints.h"
#include <stdio.h>

#define ALPHA 36 //Number of characters in alphabet
typedef struct _hash_gen_t {
    int kgram_len;
    int prime;
    int h;
    hash_t curr_hash;
} hash_gen_t;

hash_t create_generator(hash_gen_t** generator, int kgram_len, int prime, char* first_kgram);

hash_t generate_next_hash(hash_gen_t* generator, char* new_kgram);