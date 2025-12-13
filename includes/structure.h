#ifndef STRUCTURE_H
#define STRUCTURE_H

#include "vector.h"
#include <stddef.h>

vec_pointer create_structure();

vec_pointer get_source(vec_pointer structure, v_string source);

void add_source(vec_pointer structure, v_string source);

v_string get_destination(vec_pointer structure, v_string source, v_string destination);

void add_destination(vec_pointer structure, v_string source, v_string destination);

void remove_destination(vec_pointer structure, v_string source, v_string destination);

void remove_source(vec_pointer structure, v_string source);

void delete_structure(vec_pointer structure);

#endif // STRUCTURE_H