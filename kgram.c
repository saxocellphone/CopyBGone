#include "kgram.h"

void create_kgram_generator(kgram_gen_t** generator, char* source_file, char* base_text, long start_position, long end_position, int k){
    *generator = (kgram_gen_t*) malloc(sizeof(struct _kgram_gen_t));
    //The k - 1 is for next rank stuff
    long total_length = end_position - start_position + k;
    char* new_base_text = (char*) malloc(total_length * sizeof(char) + 1);
    memcpy(new_base_text, &base_text[start_position], total_length);
    new_base_text[total_length] = '\0';
    long position = 0;
    (*generator)->source_file = source_file;
    (*generator)->base_text = new_base_text;
    (*generator)->start_position = start_position;
    (*generator)->end_position = end_position;
    (*generator)->k = k;
    while(new_base_text[position] == 32 && position < end_position){
        //For the first k-gram, skip until the first character is found,
        position++;
    }
    (*generator)->position = position;
}

//Returns 1 if the generator reaches the end
int generate_next_kgram(kgram_gen_t* generator, kgram_t** kgram_buffer){
    // TODO: Make sure to free previous kgram before creating new ones.
    // Right now, you have to manually free it
    char* kgram_to_copy = &generator->base_text[generator->position];
    if(kgram_to_copy[0] == 32){
        //Skip spaces
        generator->position++;
        return 1;
    }
    if(generator->position < generator->end_position){
        *kgram_buffer = (kgram_t*) malloc(sizeof(struct _kgram_t));
        (*kgram_buffer)->kgram = (char*) malloc(generator->k * sizeof(char));
        location_t* loc = (location_t*) malloc(sizeof(struct _location_t));
        loc->pos = generator->position + generator->start_position;
        loc->source_file = generator->source_file;
        (*kgram_buffer)->location = loc;
        generator->position++;

        int counter = 0;
        int buffer_index = 0;
        while(buffer_index < generator->k){
            if(kgram_to_copy[counter] != 32){
                (*kgram_buffer)->kgram[buffer_index] = kgram_to_copy[counter];
                buffer_index++;
            }
            counter++;
        }

        if(strlen(kgram_to_copy) < generator->k){
            return 1;
        }
        (*kgram_buffer)->kgram[generator->k] = '\0';

        return 0;
    }
    return 1;
}