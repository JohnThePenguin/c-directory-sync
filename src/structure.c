#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../includes/structure.h"
#include "../includes/vector.h"


const source_t SOURCE_NOT_FOUND = {NULL, NULL, -1, true};
const dest_t DESTINATION_NOT_FOUND = {NULL, -1, true};

vec_pointer create_structure(){
    return vector_create(sizeof(source_t));
}

source_t get_source(vec_pointer structure, v_string source){
    for(int i = 0; i < structure->size; i++){
        source_t current = v_get_source(structure, i);
        if(strcmp(source->value, current.source->value) == 0){
            return current;
        }
    }
    return SOURCE_NOT_FOUND;
}

void add_source(vec_pointer structure, v_string source, pid_t pid, int pipe_fd){
    if(!get_source(structure, source).not_found) return;

    source_t new_source;
    // vector_copy(&new_source.source, source);
    new_source.source = source;
    new_source.destinations = vector_create(sizeof(dest_t));
    new_source.pipe_fd = pipe_fd;
    new_source.not_found = false;

    vector_push(structure, &new_source);

    printf("Added source to structure: %s\n", v_get_source(structure, structure->size - 1).source->value);
}

dest_t get_destination(vec_pointer structure, v_string source, v_string destination){
    source_t src_vec = get_source(structure, source);
    if(src_vec.not_found) return DESTINATION_NOT_FOUND;

    for(int i = 1; i < src_vec.destinations->size; i++){
        dest_t current = v_get_destination(src_vec.destinations, i);
        if(strcmp(destination->value, current.destination->value) == 0){
            return current;
        }
    }
    return DESTINATION_NOT_FOUND;
}

void add_destination(vec_pointer structure, v_string source, v_string destination, pid_t pid){
    source_t src_vec = get_source(structure, source);
    if(src_vec.not_found) return; // Throw error?

    if(!get_destination(structure, source, destination).not_found) return;

    dest_t new_destination;
    new_destination.pid = pid;
    // vector_copy(&new_destination.destination, destination);
    new_destination.destination = destination;
    new_destination.not_found = false;

    vector_push(src_vec.destinations, &new_destination);
}

void remove_destination(vec_pointer structure, v_string source, v_string destination){
    source_t src_vec = get_source(structure, source);
    if(src_vec.not_found) return; // Throw error?

    int i = 1;
    while(i < src_vec.destinations->size){
        dest_t dest = v_get_destination(src_vec.destinations, i);
        if(strcmp(dest.destination->value, destination->value) == 0){
            break;
        } 
        i++;
    }

    if(i == src_vec.destinations->size) return; // Throw error?
    vector_delete(v_get_destination(src_vec.destinations, i).destination);

    while(i < src_vec.destinations->size - 1){
        dest_t dest = v_get_destination(src_vec.destinations, i + 1);
        vector_set(src_vec.destinations, i, &dest);
        i++;
    }

    vector_pop(src_vec.destinations);
}
   

void remove_source(vec_pointer structure, v_string source){
    int i = 0;

    printf("Structure size before removing source: %d\n", structure->size);
    while(i < structure->size){
        source_t current = v_get_source(structure, i);

        if(strcmp(current.source->value, source->value) == 0){
            break;
        }
        i++;
    }

    if(i == structure->size) return; // Throw error?
    source_t src_vec = v_get_source(structure, i);

    while(src_vec.destinations->size > 0){
        dest_t dest_vec = v_get_destination(src_vec.destinations, src_vec.destinations->size - 1);
        printf("Removing destination: %s\n", dest_vec.destination->value);
        vector_delete(dest_vec.destination);
        vector_pop(src_vec.destinations);
    }

    vector_delete(src_vec.destinations);
    while(i < structure->size - 1){
        vector_set(structure, i, v_get_vec(structure, i + 1));
        i++;
    }

    vector_pop(structure);
}


void delete_structure(vec_pointer structure){
    printf("Deleting structure contents of size = %d\n", structure->size);

    while(structure->size > 0){
        source_t source = v_get_source(structure, structure->size - 1);
        v_string source_name = source.source;

        printf("Removing source: %s\n", source_name->value);
        remove_source(structure, source_name);
    }
    vector_delete(structure);
}


source_t v_get_source(vec_pointer v, int index){
    return v_get(source_t, v, index);
}

dest_t v_get_destination(vec_pointer v, int index){
    return v_get(dest_t, v, index);
}