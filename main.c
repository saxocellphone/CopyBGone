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
int num_threads = 2;

typedef struct _chunk_data_t {
    char* buffer;
    //Might delete this later
    char* next_chunk_buffer;
} chunk_data_t;

typedef struct {
    int thread_num;
    char* text;
    fingerprint_t* fingerprints;
    table_t* fingerprints_db;
    long start;
    long finish;
} thread_args;

void* generate_fingerprint(void* arg);
void* winnow(void* arg);

int main(int argc, char** argv){
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);
    if(mpi_myrank == 0){
        //Getting all arguments
        //TODO: Currently there are no safety checks
        for (int i = 1; i < 5; i++) {
            if (strcmp(argv[i], "--window") == 0 || strcmp(argv[i], "-w") == 0) {
                window_size = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "--kgram") == 0 || strcmp(argv[i], "-k") == 0) {
                k_gram_size = atoi(argv[i+1]);
            }
        }
        file_names = (char**) malloc(sizeof(char*) * 50 * (argc - 4));
        //Loop for filenames
        for(int i = 5; i < argc; i++){
            file_names[i-5] = argv[i];
        }
    }
    MPI_Bcast(&window_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k_gram_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
    long int filesize;
    if(mpi_myrank == 0){
        FILE* fp = fopen(file_names[0], "r");
        fseek(fp, 0L, SEEK_END); 
        filesize = ftell(fp); 
        printf("Filesize of %s: %ld\n", file_names[0], filesize);
        fclose(fp);
    }
    //Collective operation to get total filesize.
    MPI_Bcast(&filesize, 1, MPI_LONG, 0, MPI_COMM_WORLD);
    //TODO: broadcast file names to all ranks
    chars_per_chunk = filesize / mpi_commsize;
    next_buffer_count = k_gram_size - 1;
    //Allocate the local chunk of data
    chunk_data_t* local_chunk = (chunk_data_t*) malloc(sizeof(struct _chunk_data_t));
    local_chunk->buffer = (char*) malloc(sizeof(char) * chars_per_chunk + next_buffer_count);
    // local_chunk->next_chunk_buffer = (char*) malloc(sizeof(char) * next_buffer_count);
    MPI_File * fh = malloc(sizeof * fh);
    //Opens file, right now it's only doing one file.
    MPI_File_open(MPI_COMM_WORLD, "test_original.txt", MPI_MODE_RDONLY, MPI_INFO_NULL, fh);
    //Read file at the correct location.
    MPI_File_read_at(*fh, mpi_myrank * chars_per_chunk, local_chunk->buffer, filesize / mpi_commsize,
                  MPI_CHAR, MPI_STATUS_IGNORE);

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
    fingerprint_t* fingerprints = (fingerprint_t*) calloc(chars_per_chunk, sizeof(struct _fingerprint_t));

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
        arg->fingerprints = fingerprints;
        arg->start = i * chars_per_thread;
        arg->finish = (i+1) * chars_per_thread;
        pthread_create(&threads[i], NULL, generate_fingerprint, (void*) arg);
    }

    for (int j = 0; j < num_threads; j++) {
        void * ret;
        pthread_join(threads[j], &ret);
    }

    /****************************************************
     *  Multithreading for winnowing
     * **************************************************/
    // initialize fingerprint database
    table_t* fingerprints_db;
    fingerprints_create(&fingerprints_db, PRIME);

    for(int i = 0; i < num_threads; i++){
        thread_args* arg = (thread_args*) malloc(sizeof(thread_args));
        arg->fingerprints = fingerprints;
        arg->fingerprints_db = fingerprints_db;
        arg->start = i * chars_per_thread;
        arg->finish = (i+1) * chars_per_thread;
        arg->thread_num = i;
        pthread_create(&threads[i], NULL, winnow, (void*) arg);
    }

    for (int j = 0; j < num_threads; j++) {
        void * ret;
        pthread_join(threads[j], &ret);
    }

    // location_list_t* locations_found;
    // int status = fingerprints_get(fingerprints_db, 184, &locations_found);
    // printf("Status: %d\n", status);
    // printf("Found hash 184 at %d locations: \n", locations_found->size);
    // location_node_t* curr = locations_found->head;
    // for(int i = 0; i < locations_found->size; i++){
    //     printf("Position: %ld, Src: %s\n", curr->location.pos, curr->location.source_file);
    //     curr = curr->next;
    // }
    free(fingerprints_db->buckets);
    free(fingerprints_db);
    free(local_chunk->buffer);
    // free(local_chunk->next_chunk_buffer);
    free(local_chunk);
    free(file_names);
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
}

void* winnow(void* arg){
    //An array of potential fingerprints
    fingerprint_t* fingerprints = ((thread_args*) arg)->fingerprints;
    int r = 0; // index to window
    int min = 0; // index of minimum element in window
    fingerprint_t h[window_size]; // rolling window of hashes
    // Initialize the window
    for(int i = 0; i < window_size; i++){
        h[i].hash = INT_MAX;
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
        h[r] = fingerprints[i];

        if(min == r){
            min = 0;
            for(int j = (r-1) % window_size; j != r; j = (j-1+window_size) % window_size){
                if(h[j].hash < h[min].hash){
                    min == j;
                }
            }
            // printf("Out of window: Hash: %d, Position: %ld, Min: %d\n", h[min].hash, h[min].location.pos, min);
            fingerprints_add(((thread_args*) arg)->fingerprints_db, h[min].hash, h[min].location);
        } else {
            if(h[r].hash <= h[min].hash) {
                // printf("In window: Hash: %d, Position: %ld\n", h[min].hash, h[min].location.pos);
                min = r;
                // printf("In window: Hash: %d, Position: %ld, Min: %d\n", h[min].hash, h[min].location.pos, min);
                fingerprints_add(((thread_args*) arg)->fingerprints_db, h[min].hash, h[min].location);
            }
        }
    }
}

void* generate_fingerprint(void* arg){
    //right now the first k-gram and hash is being calculated seperately. Might refactor this
    kgram_gen_t* kgram_gen;
    create_kgram_generator(&kgram_gen, "test_original.txt", ((thread_args*) arg)->text, ((thread_args*) arg)->start, ((thread_args*) arg)->finish, k_gram_size);
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
            ((thread_args*) arg)->fingerprints[i] = fingerprint;
        }
    }
    free(kgram_gen);
    free(hash_gen);
}
