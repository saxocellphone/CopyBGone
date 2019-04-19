#include "kgram.h"

void create_kgram_generator(kgram_gen_t** generator, char* base_text, int base_text_len, int k){
    *generator = (kgram_gen_t*) malloc(sizeof(struct _kgram_gen_t));
    char* new_base_text = (char*) malloc(base_text_len * sizeof(char));
    int new_text_counter = 0;
    for(int i = 0; i < base_text_len; i++){
        int ascii = base_text[i];
        if(ascii == 32){
            //Skips spaces, may add more in the future
            continue;
        }
        new_base_text[new_text_counter] = base_text[i];
        new_text_counter++;
    }
    (*generator)->base_text = new_base_text;
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