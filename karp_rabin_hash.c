#include "karp_rabin_hash.h"

int int_pow(int base, int exp);

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
        first_hash += (first_kgram[i] - 'a' + 1) * int_pow(ALPHA, k - i - 1);
    }
    (*generator)->curr_hash = first_hash;
    (*generator)->curr_kgram = first_kgram;
    return first_hash;
}

/**
 * Generate the next hash for k-gram based on the previous hash (BLOCKCHAIN?)
 * */
hash_t generate_next_hash(hash_gen_t* generator, char* new_kgram){
    //Using the current k-gram hash to generate the next k-gram hash
    hash_t new_hash = generator->curr_hash - ((generator->curr_kgram[0] - 'a' + 1) * int_pow(ALPHA, generator->kgram_len -1));
    new_hash *= ALPHA;
    new_hash += new_kgram[generator->kgram_len-1] - 'a' + 1;
    // printf("curr_kgram: %s, new_kgram: %s, curr_hash: %d, new_hash: %d, thing: %d\n", generator->curr_kgram, new_kgram, generator->curr_hash, new_hash, ((generator->curr_kgram[0] - 'a' + 1) * int_pow(ALPHA, generator->kgram_len -1)));
    generator->curr_hash = new_hash;
    generator->curr_kgram = new_kgram;
    return new_hash % generator->prime;
}

int int_pow(int base, int exp)
{
    int result = 1;
    while (exp)
    {
        if (exp & 1)
           result *= base;
        exp /= 2;
        base *= base;
    }
    return result;
}