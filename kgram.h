#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Kgram generator generates k-gram with size k from a given text.
 * To get the next k-gram, just simply call generate_next_kgram.
 * */

//Kgram type with location information.
typedef struct _kgram_t{
    char* kgram;
    int location;
} kgram_t;
//The kgram generator type.
typedef struct _kgram_gen_t {
    char* base_text;
    int position;
    int k;
} kgram_gen_t;

void create_kgram_generator(kgram_gen_t** generator, char* base_text, int base_text_len, int k);
void generate_next_kgram(kgram_gen_t* generator, kgram_t** kgram_buffer);