#ifndef GUARD_types_h
#define GUARD_types_h

typedef unsigned int hash_t;
typedef long position_t;
typedef struct _location_t {
    position_t pos;
    char * source_file;
} location_t;

#endif