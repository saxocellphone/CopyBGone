#include <stdio.h>
#include <stdlib.h>
#include<string.h>
#include<mpi.h>

#include "fingerprints.h"
#include "karp_rabin_hash.h"
//MPI Inits
int mpi_commsize, mpi_myrank;
//Arg inits
int window_size, k_gram_size;
char** file_names;

int main(int argc, char** argv){
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &mpi_commsize);
    MPI_Comm_rank( MPI_COMM_WORLD, &mpi_myrank);
    if(mpi_myrank == 0){
        //Getting all arguments
        //TODO: Currently there are no safety checks
        for (int i = 0; i < 2; i++) {
            if (strcmp(argv[i], "--window") == 0 || strcmp(argv[i], "-w") == 0) {
                window_size = atoi(argv[++i]);
            }
            if (strcmp(argv[i], "--kgram") == 0 || strcmp(argv[i], "-k") == 0) {
                k_gram_size = atoi(argv[++i]);
            }
        }
        file_names = (char**) malloc(sizeof(char*) * 50 * (argc - 4));
        //Loop for filenames
        for(int i = 5; i < argc; i++){
            file_names[i-5] = argv[i];
        }
    }


    if(mpi_myrank == 0){
        FILE* fp = fopen(file_names[0], "r");
        fseek(fp, 0L, SEEK_END);   
        // calculating the size of the file 
        long int res = ftell(fp); 
        // closing the file 
        fclose(fp); 
        printf("%ld\n", res);
    }
    MPI_Barrier( MPI_COMM_WORLD );
    MPI_Finalize();
}
