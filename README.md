# Copy🅱️Gone
A tool that helps detect plagiarism among documents using MPI and pThreads. 

# To Compile:
`mpicc *.c -o copyBGone`

# Usage:
`mpirun -np <num-ranks> ./copyBGone [-k|--kgram <num-kgrams>] [-t|--threads <num-threads-per-rank>] [-w|--window <window-size>] file1 [file2...]`

# Future work:
Currently, this program only works with text documents. The goal is to generalize it and use it with code, images, and even audio. To accomplish that, tokenizers are necessary to parse the data.
