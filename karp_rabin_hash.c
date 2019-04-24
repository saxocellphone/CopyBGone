#include "karp_rabin_hash.h"

/**
 * Creates a generator of hashes for k-grams. 
 * */
hash_t create_hash_generator(hash_gen_t** generator, int k, long prime, char* first_kgram){
    *generator = (hash_gen_t*) malloc(sizeof(struct _hash_gen_t));
    int h = 1;
    (*generator)->prime = prime;
    (*generator)->kgram_len = k;
    //Loop for calculating h, the common base-factor
    for(int i = 0; i < k - 1; i++){
        h = (h * ALPHA) % prime;
    }
    (*generator)->h = h;
    hash_t first_hash = 0;
    //Generating the first k-gram
    for(int i = 0; i < k; i++){
        first_hash = (ALPHA * first_hash + first_kgram[i]) % prime;
    }
    (*generator)->curr_hash = first_hash;
    return first_hash;
}

/**
 * Generate the next hash for k-gram based on the previous hash (BLOCKCHAIN?)
 * */
hash_t generate_next_hash(hash_gen_t* generator, char* new_kgram){
    //Using the current k-gram hash to generate the next k-gram hash
    hash_t new_hash = (ALPHA * (generator->curr_hash - new_kgram[0] * generator->h) + 
                      new_kgram[generator->kgram_len-1]) % generator->prime;
    return new_hash;
}