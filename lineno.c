#include "lineno.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>

#define MAXLINE 10000
struct _lineno_node_t {
    position_t first_char;
    struct _lineno_node_t * next;
};

size_t read_lineno(char * fd, position_t ** lineno) {
    FILE * fp = fopen(fd, "r");
    if (!fp) {
        printf("Failed\n");
        exit(1);
    }

    struct _lineno_node_t * ptr = 
        (struct _lineno_node_t *) malloc(sizeof(struct _lineno_node_t));
    ptr->first_char = 0;
    ptr->next = NULL;
    struct _lineno_node_t * head = ptr;

    size_t size = 1;
    size_t pos = 0;


    while (!feof(fp)) {
        char c = fgetc(fp);
        ++pos;
        if (c == '\n' || c == EOF) {
            struct _lineno_node_t * next = 
                (struct _lineno_node_t *) malloc(sizeof(struct _lineno_node_t));
            next->first_char = pos;
            next->next = NULL;
            ptr->next = next;
            ptr = next;

            ++size;
        }

    }

    // copy linked list into array
    *lineno = 
        (position_t *) malloc(size * sizeof(struct _lineno_node_t));

    ptr = head;

    for (size_t j = 0; j < size; j++, ptr=ptr->next) {
        (*lineno)[j] = ptr->first_char;
    }

    return size;
}

void get_lineno(position_t * lineno, size_t len, position_t pos,
                  size_t * row, size_t * col) {
    size_t L = 0,
           R = len - 1,
           m;
    *row = -1;
    *col = -1;
    while (L <= R) {
        m = (L + R) / 2;

        if (lineno[L] < pos && (L == len - 1 || pos < lineno[L + 1])) {
            *row = L;
            break;
        } 
        if (pos < lineno[R] && (R == 0 || lineno[R - 1] < pos)) {
            *row =  R - 1;
            break;
        }

        if (lineno[m] < pos) {
            L = m + 1;
        } else if (lineno[m] > pos) {
            R = m - 1;
        } else {
            *row = m;
            break;
        }
    }
    
    if (*row != -1) {
        *col = pos - lineno[*row];
    }
}