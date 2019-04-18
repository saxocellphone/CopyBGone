#include <stdlib.h>
#include <string.h>
#include <stdio.h>
typedef struct _kgram_t{
    char* kgram;
    int location;
} kgram_t;
typedef struct _kgram_gen_t {
    char* base_text;
    int position;
    int k;
} kgram_gen_t;

void create_kgram_generator(kgram_gen_t** generator, char* base_text, int k);
void generate_next_kgram(kgram_gen_t* generator, kgram_t** kgram_buffer);