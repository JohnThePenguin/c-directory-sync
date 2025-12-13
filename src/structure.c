#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../includes/structure.h"
#include "../includes/vector.h"

vec_pointer create_structure(){
    return vector_create(sizeof(vec_pointer));
}

vec_pointer get_source(vec_pointer structure, v_string source){
    for(int i = 0; i < structure->size; i++){
        v_string current = v_get_vec(structure, i);
        v_string first = v_get_vec(current, 0);
        if(current->size >= 1 && strcmp(source->value, first->value) == 0){
          return current;
        }
    }
    return NULL;
}

void add_source(vec_pointer structure, v_string source){
    if(get_source(structure, source)) return;
        
    vec_pointer new_source = vector_create(sizeof(v_string));
    vector_push(new_source, &source);

    printf("Added source to source: %d\n", new_source->size);

    vector_push(structure, &new_source);
}

v_string get_destination(vec_pointer structure, v_string source, v_string destination){
    vec_pointer src_vec = get_source(structure, source);
    if(src_vec == NULL) return NULL;

    for(int i = 1; i < src_vec->size; i++){
        v_string current = v_get_vec(src_vec, i);
        if(strcmp(destination->value, current->value) == 0){
            return current;
        }
    }
    return NULL;
}

void add_destination(vec_pointer structure, v_string source, v_string destination){
    vec_pointer src_vec = get_source(structure, source);
    if(src_vec == NULL) return; // Throw error?

    if(get_destination(structure, source, destination)) return;
    vector_push(src_vec, &destination);
}

void remove_destination(vec_pointer structure, v_string source, v_string destination){
    vec_pointer src_vec = get_source(structure, source);
    if(src_vec == NULL) return; // Throw error?

    int i = 1;
    while(i < src_vec->size){
        v_string dest = v_get_vec(src_vec, i);
        if(strcmp(dest->value, destination->value) == 0){
            break;
        } 
        i++;
    }

    if(i == src_vec->size) return; // Throw error?
    vector_delete(v_get_vec(src_vec, i));

    while(i < src_vec->size - 1){
        vec_pointer dest = v_get_vec(src_vec, i + 1);
        vector_set(src_vec, i, &dest);
        i++;
    }

    vector_pop(src_vec);
}
   

void remove_source(vec_pointer structure, v_string source){
    int i = 0;

    printf("Structure size before removing source: %d\n", structure->size);
    while(i < structure->size){
        v_string current = v_get_vec(structure, i);
        v_string first = v_get_vec(current, 0);

        if(strcmp(first->value, source->value) == 0){
            break;
        }
        i++;
    }

    if(i == structure->size) return; // Throw error?
    vec_pointer src_vec = v_get_vec(structure, i);

    while(src_vec->size > 0){
        v_string dest_vec = v_get_vec(src_vec, src_vec->size - 1);

        printf("Removing destination: %s\n", dest_vec->value);
        vector_delete(dest_vec);
        vector_pop(src_vec);
    }

    vector_delete(src_vec);
    while(i < structure->size - 1){
        vector_set(structure, i, v_get_vec(structure, i + 1));
        i++;
    }

    vector_pop(structure);
}


void delete_structure(vec_pointer structure){
    printf("Deleting structure contents of size = %d\n", structure->size);

    while(structure->size > 0){
        vec_pointer source = v_get_vec(structure, structure->size - 1);
        v_string source_name = v_get_vec(source, 0);

        printf("Removing source: %s\n", source_name->value);
        remove_source(structure, source_name);
    }
    vector_delete(structure);
}