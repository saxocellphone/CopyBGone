#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <pthread.h>
#include <limits.h>

#include "fingerprints.h"
#include "karp_rabin_hash.h"
#include "kgram.h"

#define PRIME 105943
//MPI Inits
int mpi_commsize, mpi_myrank;
//Arg inits
int window_size, k_gram_size;
char** file_names;
//Global vars
int chars_per_chunk;
int next_buffer_count; //This is the count for the carry-over from the next rank
//Thread stuff
int num_threads = 4;

typedef struct _chunk_data_t {
    char* buffer;
} chunk_data_t;

typedef struct {
    int thread_num;
    char* text;
    fingerprint_t* unwinnowed_fingerprints;
    table_t* fingerprints_db;
    fingerprint_t** winnowed_fingerprints;
    char* file_name;
    long start;
    long finish;
} thread_args;

void* generate_fingerprint(void* arg);
void* winnow(void* arg);

int main(int argc, char** argv){
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);

    for (int i = 1; i < 5; i++) {
        if (strcmp(argv[i], "--window") == 0 || strcmp(argv[i], "-w") == 0) {
            window_size = atoi(argv[i+1]);
        }
        if (strcmp(argv[i], "--kgram") == 0 || strcmp(argv[i], "-k") == 0) {
            k_gram_size = atoi(argv[i+1]);
        }
    }
    file_names = (char**) malloc(sizeof(char*) * 50 * (argc - 4));
    int num_files = argc - 5;
    //Loop for filenames, starts at 5 because that's where the filename starts
    for(int i = 0; i < num_files; i++){
        file_names[i] = argv[i + 5];
    }
    
    long int filesize;
    table_t* fingerprints_db; //Stores all winnowed fingerprints
    fingerprints_create(&fingerprints_db, PRIME);
    chunk_data_t* local_chunk = (chunk_data_t*) malloc(sizeof(struct _chunk_data_t)); //Allocate the local chunk of data
    for(int n = 0; n < num_files; n++){
        if(mpi_myrank == 0){
            FILE* fp = fopen(file_names[n], "r");
            fseek(fp, 0L, SEEK_END); 
            filesize = ftell(fp); 
            printf("Filesize of %s: %ld\n", file_names[n], filesize);
            fclose(fp);
        }
        //Collective operation to get total filesize.
        MPI_Bcast(&filesize, 1, MPI_LONG, 0, MPI_COMM_WORLD);
        chars_per_chunk = filesize / mpi_commsize;
        next_buffer_count = k_gram_size - 1;
        local_chunk->buffer = (char*) malloc(sizeof(char) * chars_per_chunk + next_buffer_count);
        MPI_File * fh = malloc(sizeof * fh);
        //Opens file, right now it's only doing one file.
        MPI_File_open(MPI_COMM_WORLD, file_names[n], MPI_MODE_RDONLY, MPI_INFO_NULL, fh);
        //Read file at the correct location.
        MPI_File_read_at(*fh, mpi_myrank * chars_per_chunk, local_chunk->buffer, chars_per_chunk, MPI_CHAR, MPI_STATUS_IGNORE);
        MPI_File_close(fh);
        free(fh);
        MPI_Request send_req, recv_req;
        //Sends the nessesary k-gram from next rank to previous
        if(mpi_myrank != mpi_commsize - 1){
            // all but last rank will receive from next rank
            MPI_Irecv(&local_chunk->buffer[chars_per_chunk], next_buffer_count, MPI_CHAR, mpi_myrank + 1, 0, MPI_COMM_WORLD, &recv_req);
            MPI_Wait(&recv_req, MPI_STATUS_IGNORE);    }
        if(mpi_myrank != 0){
            // all but first rank will send to next rank
            MPI_Isend(local_chunk->buffer, next_buffer_count, MPI_CHAR, mpi_myrank - 1, 0, MPI_COMM_WORLD, &send_req);
        }
        // allocate space for all candidate fingerprints
        fingerprint_t* unwinnowed_fingerprints = (fingerprint_t*) calloc(chars_per_chunk, sizeof(struct _fingerprint_t));

        /****************************************************
         *  Multithreading for kgram and hashes
         * **************************************************/
        //Generates fingerprints and hashes and store them in fingerprints
        pthread_t threads[num_threads];
        long chars_per_thread = chars_per_chunk / num_threads;
        for(int i = 0; i < num_threads; i++){
            //All the args to path to pthread are in this struct
            thread_args* arg = (thread_args*) malloc(sizeof(thread_args));
            arg->thread_num = i;
            arg->text = local_chunk->buffer;
            arg->unwinnowed_fingerprints = unwinnowed_fingerprints;
            arg->start = i * chars_per_thread;
            arg->finish = (i+1) * chars_per_thread;
            arg->file_name = file_names[n];
            pthread_create(&threads[i], NULL, generate_fingerprint, (void*) arg);
        }

        for (int j = 0; j < num_threads; j++) {
            void * ret;
            pthread_join(threads[j], &ret);
            free(ret);
        }
        /****************************************************
         *  Multithreading for winnowing
         * **************************************************/
        //This is double pointer because it stores the pointer to the fingerprints in
        //unwinnowed_fingerprints
        //TODO: Switch this to an array of pointers
        fingerprint_t** winnowed_fingerprints = (fingerprint_t**) calloc(chars_per_chunk, sizeof(fingerprint_t*));
        for(int i = 0; i < num_threads; i++){
            thread_args* arg = (thread_args*) malloc(sizeof(thread_args));
            arg->unwinnowed_fingerprints = unwinnowed_fingerprints;
            arg->winnowed_fingerprints = winnowed_fingerprints;
            arg->fingerprints_db = fingerprints_db;
            arg->start = i * chars_per_thread;
            arg->finish = (i+1) * chars_per_thread;
            arg->thread_num = i;
            pthread_create(&threads[i], NULL, winnow, (void*) arg);
        }

        for (int j = 0; j < num_threads; j++) {
            void * ret;
            pthread_join(threads[j], &ret);
            free(ret);
        }

        /****************************************************
         * adding fingerprint to DB and querying across ranks
         * **************************************************/
        hash_t query[mpi_commsize];
        for(int i = 0; i < chars_per_chunk - k_gram_size - 1; i++){
            if(winnowed_fingerprints[i] == NULL){
                continue;
            }
            MPI_Scatter(&winnowed_fingerprints[i]->hash, 1, MPI_INT, 
                &query[mpi_myrank], 1, MPI_INT, mpi_myrank, MPI_COMM_WORLD);
            location_list_t* locations_found;
            int status = fingerprints_get(fingerprints_db, winnowed_fingerprints[i]->hash, &locations_found);
            if(status == 1){
                printf("Found hash %d at %d locations: \n", winnowed_fingerprints[i]->hash, locations_found->size);
                location_node_t* curr = locations_found->head;
                for(int i = 0; i < locations_found->size; i++){
                    printf("Position: %ld, Src: %s\n", curr->location.pos, curr->location.source_file);
                    curr = curr->next;
                }
            } else {
                fingerprints_add(fingerprints_db, winnowed_fingerprints[i]->hash, winnowed_fingerprints[i]->location);
            }
        }
        free(local_chunk->buffer);
        free(unwinnowed_fingerprints);
    }
    

    free(fingerprints_db->buckets);
    free(fingerprints_db);
    free(local_chunk);
    free(file_names);
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
}

void* winnow(void* arg){
    //An array of potential fingerprints
    fingerprint_t* fingerprints = ((thread_args*) arg)->unwinnowed_fingerprints;
    int r = 0; // index to window
    int min = 0; // index of minimum element in window
    fingerprint_t** h = (fingerprint_t**) malloc(window_size * sizeof(fingerprint_t*)); // rolling window of hashes
    // Initialize the window
    for(int i = 0; i < window_size; i++){
        fingerprint_t* f = (fingerprint_t*) malloc(sizeof(fingerprint_t));
        f->hash = INT_MAX;
        h[i] = f;
    }

    for(int i = ((thread_args*) arg)->start; i < ((thread_args*) arg)->finish; i++){
        if(i >= chars_per_chunk - k_gram_size - 1){
            //Protection against index out of bound
            break;
        }
        if(fingerprints[i].hash == 0){
            continue;
        }
        r = (r + 1) % window_size;
        h[r] = &fingerprints[i];
        int status = -1;
        if(min == r){
            min = 0;
            for(int j = (r-1) % window_size; j != r; j = (j-1+window_size) % window_size){
                if(h[j]->hash < h[min]->hash){
                    min == j;
                }
            }
            ((thread_args*) arg)->winnowed_fingerprints[i] = h[min];
        } else {
            if(h[r]->hash <= h[min]->hash) {
                min = r;
                ((thread_args*) arg)->winnowed_fingerprints[i] = h[min];
            }
        }
    }

    return arg;
}

void* generate_fingerprint(void* arg){
    //right now the first k-gram and hash is being calculated seperately. Might refactor this
    kgram_gen_t* kgram_gen;
    create_kgram_generator(&kgram_gen, ((thread_args*) arg)->file_name, ((thread_args*) arg)->text, ((thread_args*) arg)->start, ((thread_args*) arg)->finish, k_gram_size);
    kgram_t* first_kgram;
    generate_next_kgram(kgram_gen, &first_kgram);
    hash_gen_t* hash_gen;
    create_hash_generator(&hash_gen, k_gram_size, PRIME, first_kgram->kgram);

    char* text = ((thread_args*) arg)->text;
    int thread_num = ((thread_args*) arg)->thread_num;
    for(long i = ((thread_args*) arg)->start; i < ((thread_args*) arg)->finish; i++){
        kgram_t* new_kgram;
        int status = generate_next_kgram(kgram_gen, &new_kgram);
        if(status == 0){
            hash_t new_hash = generate_next_hash(hash_gen, new_kgram->kgram);
            free(new_kgram);
            fingerprint_t fingerprint;
            fingerprint.hash = new_hash;
            fingerprint.location = *new_kgram->location;
            ((thread_args*) arg)->unwinnowed_fingerprints[i] = fingerprint;
        }
    }
    free(kgram_gen);
    free(hash_gen);
    return arg;
}
