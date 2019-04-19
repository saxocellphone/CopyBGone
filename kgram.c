#include "kgram.h"

void create_kgram_generator(kgram_gen_t** generator, char* source_file, char* base_text, int base_text_len, int k){
    *generator = (kgram_gen_t*) malloc(sizeof(struct _kgram_gen_t));
    char* new_base_text = (char*) malloc(base_text_len * sizeof(char));
    unsigned int new_text_counter = 0;
    // unsigned int chars_removed = 0;
    for(int i = 0; i < base_text_len; i++){
        //TODO: Maybe make this parallel? Right now it's slow AF
        int ascii = base_text[i];
        if(ascii == 32){
            //Skips spaces, may add more in the future
            // chars_removed++;
            continue;
        }
        new_base_text[new_text_counter] = base_text[i];
        new_text_counter++;
    }
    (*generator)->source_file = source_file;
    (*generator)->base_text = new_base_text;
    (*generator)->position = 0;
    (*generator)->k = k;
}

//Returns 1 if the generator reaches the end
int generate_next_kgram(kgram_gen_t* generator, kgram_t** kgram_buffer){
    // TODO: Make sure to free previous kgram before creating new ones.
    // Right now, you have to manually free it
    char* kgram_to_copy = &generator->base_text[generator->position];
    if(strlen(kgram_to_copy) != generator->k){
        *kgram_buffer = (kgram_t*) malloc(sizeof(struct _kgram_t));
        (*kgram_buffer)->kgram = (char*) malloc(generator->k * sizeof(char));
        memcpy((*kgram_buffer)->kgram, kgram_to_copy, generator->k);
        (*kgram_buffer)->kgram[generator->k] = '\0';
        location_t loc;
        loc.pos = generator->position;
        loc.source_file = generator->source_file;
        (*kgram_buffer)->location = &loc;
        generator->position++;
        return 0;
    }
    return 1;
}