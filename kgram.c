#include "kgram.h"

void create_kgram_generator(kgram_gen_t** generator, char* base_text, int k){
    *generator = (kgram_gen_t*) malloc(sizeof(struct _kgram_gen_t));
    (*generator)->base_text = base_text;
    (*generator)->position = 0;
    (*generator)->k = k;
}

void generate_next_kgram(kgram_gen_t* generator, kgram_t** kgram_buffer){
    // TODO: Make sure to free previous kgram before creating new ones.
    // Right now, you have to manually free it
    *kgram_buffer = (kgram_t*) malloc(sizeof(struct _kgram_t));
    (*kgram_buffer)->kgram = (char*) malloc(generator->k * sizeof(char));
    memcpy((*kgram_buffer)->kgram, &generator->base_text[generator->position], generator->k);
    (*kgram_buffer)->kgram[generator->k] = '\0';
    (*kgram_buffer)->location = generator->position;
    generator->position++;
}