// Source: https://github.com/JohnThePenguin/big-calculator-c

// #include "error.h"

#include "../includes/utils.h"
#include "../includes/vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Descriptions and comments are in include/numbers_systems.h */

vec_pointer vector_create(size_t element_size){
    vec_pointer a = (vec_pointer)malloc(sizeof(struct vector));
    a->size = 0;
    a->allocated = 0;
    a->element_size = element_size;
    a->value = malloc(2 * element_size);

    if(a->value == NULL){
        error("Could not allocate memory for vector");
    }
    else{
        a->allocated = 2;
    }

    return a;
}

void vector_delete(vec_pointer v){
    if(v != NULL){
        free(v->value);
        free(v);
    }
}

void vector_copy(vec_pointer* destination, vec_pointer b){
    vector_delete(*destination);

    /* Simply copy property by property */
    (*destination) = (vec_pointer)malloc(sizeof(struct vector));
    (*destination)->size = b->size;
    (*destination)->allocated = b->allocated;
    (*destination)->value = malloc(b->allocated * b->element_size);

    /* Copy memory from b to destination */
    memcpy(
        (*destination)->value,
        b->value,
        b->allocated * b->element_size
    );
}

/* Deletes destination, overwrites what a points to and links a to destination*/
void vector_rewrite(vec_pointer* destination, vec_pointer* a){
    vector_delete(*destination);
    *destination = *a;
    *a = NULL;
}

void vector_set_size(vec_pointer v, int size){
    if(v->allocated == size){
        return;
    }
    if(v->allocated > size){
        return;
    }
    else{
        v->value = realloc(v->value, size * v->element_size);
        
        if(v->value == NULL){
            error("Could not allocate!");
        }   
        else{
            v->allocated = size;
        }
    }
}

/* Pushes new value to error, if array is too small, sets new size */
/* Resizes it to 2*[current size], so complexity is nearly linear */
int vector_push(vec_pointer v, void* element){
    if(v->allocated == v->size){
        vector_set_size(v, v->size * 2);
    }

    memcpy(
        v->value + (v->size * v->element_size),
        element,
        v->element_size
    );

    return ++v->size;   
}

void vector_set(vec_pointer v, int index, void* element){
    memcpy(
        v->value + (index * v->element_size),
        element,
        v->element_size
    );
}

int vector_pop(vec_pointer v){
    if(v->size == 0){
        return -1;
    }
    else{
        return --v->size;
    }
}

void vector_empty(vec_pointer v){
    v->size = 0;
}

int v_get_int(vec_pointer v, int index){
    return ((int*)(v->value))[index];
}

char v_get_char(vec_pointer v, int index){
    return ((char*)(v->value))[index];
}

vec_pointer v_get_vec(vec_pointer v, int index){
    return ((vec_pointer*)(v->value))[index];
}


vec_pointer vector_from_cstr(const char* cstr, ssize_t size){
    if(size < 0){
        size = strlen(cstr) + 1;
    }

    vec_pointer v = vector_create(sizeof(char));
    vector_set_size(v, size);
    memcpy(v->value, cstr, size * sizeof(char));
    return v;
}


void vector_print(vec_pointer v){
    int i = 0;

    if(v == NULL){
        // printf("<Given NULL as pointer to Vector>");
    }
    else{
        for(i = v->size - 1; i >= 0; i--){
            printf("%c ", v_get_char(v, i));
        }
    }
    printf("\n");
}