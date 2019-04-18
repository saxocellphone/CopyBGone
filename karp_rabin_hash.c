#include "karp_rabin_hash.h"

/**
 * Creates a generator of hashes for k-grams. 
 * */
hash_t create_generator(hash_gen_t** generator, int kgram_len, int prime, char* first_kgram){
    *generator = (hash_gen_t*) malloc(sizeof(struct _hash_gen_t));
    int h = 1;
    (*generator)->prime = prime;
    (*generator)->kgram_len = kgram_len;
    for(int i = 0; i < kgram_len - 1; i++){
        h = (h * ALPHA) % prime;
    }
    (*generator)->h = h;
    hash_t first_hash = 0;
    for(int i = 0; i < kgram_len; i++){
        first_hash = (ALPHA * first_hash + first_kgram[i]) % prime;
    }
    (*generator)->curr_hash = first_hash;
    return first_hash;
}

/**
 * Generate the next hash for k-gram based on the previous hash (BLOCKCHAIN?)
 * */
hash_t generate_next_hash(hash_gen_t* generator, char* new_kgram){
    hash_t new_hash = (ALPHA * (generator->curr_hash - new_kgram[0] * generator->h) + 
                      new_kgram[generator->kgram_len-1]) % generator->prime;
    return new_hash;
}