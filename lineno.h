#ifndef GUARD_lineno_h
#define GUARD_lineno_h

#include "fingerprints.h"

// read linenumbers into an array of positions
// the element lineno[i] represents the the index of 
// the first character of line number i
size_t read_lineno(char * fd, position_t ** lineno);

// binary search for the line number of 
size_t get_lineno(position_t * lineno, size_t len, position_t pos);

#endif