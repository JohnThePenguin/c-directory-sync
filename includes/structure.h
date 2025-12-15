#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "vector.h"
#include <stddef.h>
#include <stdbool.h>

struct source_structure {
    v_string source;
    vec_pointer destinations; // vector of v_string

    int key;
    long pipe_wr_fd;

    bool not_found;
};

struct destination_structure {
    v_string destination;

    int key;

    bool not_found;
};

typedef struct source_structure source_t;
typedef struct destination_structure dest_t;

extern const source_t SOURCE_NOT_FOUND;
extern const dest_t DESTINATION_NOT_FOUND;

vec_pointer create_structure();

source_t get_source(vec_pointer structure, v_string source);

void add_source(vec_pointer structure, v_string source, int key, int pipe_wr_fd);

dest_t get_destination(vec_pointer structure, v_string source, v_string destination);

void add_destination(vec_pointer structure, v_string source, v_string destination, int key);

void remove_destination(vec_pointer structure, v_string source, v_string destination);

void remove_source(vec_pointer structure, v_string source);

void delete_structure(vec_pointer structure);

void printf_structure(vec_pointer structure);

source_t v_get_source(vec_pointer v, int index);

dest_t v_get_destination(vec_pointer v, int index);

#endif // STRUCTURE_H