#ifndef GUARD_KGRAM_H
#define GUARD_KGRAM_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "fingerprints.h"
/**
 * Kgram generator generates k-gram with size k from a given text.
 * To get the next k-gram, just simply call generate_next_kgram.
 * */

//Kgram type with location information.
typedef struct _kgram_t{
    char* kgram;
    location_t* location;
} kgram_t;
//The kgram generator type.
typedef struct _kgram_gen_t {
    char* source_file;
    char* base_text;
    long position;
    //The position in the sub-chunk data
    long start_position;
    long end_position;
    int k;
} kgram_gen_t;

void create_kgram_generator(kgram_gen_t** generator, char* source_file, char* base_text, long start_position, long end_position, int k);

int generate_next_kgram(kgram_gen_t* generator, kgram_t** kgram_buffer);
#endif