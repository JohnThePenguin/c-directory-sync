/*
    File: ivector.h
    Description:
        This file contains the definition of
        the Vector struct (like c++) and its functions.
        Vector's elements are of type void*.
    Source: https://github.com/JohnThePenguin/big-calculator-c
*/

#ifndef VECTOR_H
#define VECTOR_H

#include <stddef.h>
#include <sys/types.h>

/*
    Vector structure
    value - void* array of elements
    size - current use size of array
    allocated - current allocated size of array
*/
struct vector
{
    void* value;
    int size;
    int allocated;
    size_t element_size;
};

typedef struct vector* vec_pointer;
typedef struct vector* v_string;

/*
    Function creates vector
    @return pointer to vector
*/
vec_pointer vector_create(size_t element_size);

/*
    Function deletes vector
    @param v - pointer to vector
    @return void
*/
void vector_delete(vec_pointer v);

/*
    Function copies vector b to vector destination
    and deletes original value of destination vector
    @param destination - pointer to vector
    @param b - pointer to vector
    @return void
*/
void vector_copy(vec_pointer* destination, vec_pointer b);

/*
    Function rewrites vector a to vector destination
    and deletes original value of destination vector and second vector
    @param destination - pointer to vector
    @param a - pointer to vector
    @return void
*/
void vector_rewrite(vec_pointer* destination, vec_pointer* a);

/*
    Function sets vector size
    @param v - pointer to vector
    @param size - new size of vector as integer
    @return void
*/
void vector_set_size(vec_pointer v, int size);

/*
    Function pushes void* element to the end of vector
    and resizes vector if needed
    @param v - pointer to vector
    @param element - void* element to be pushed
    @return void
*/
int vector_push(vec_pointer v, void* element);

/*
    Function sets void* element at given index in vector
    @param v - pointer to vector
    @param index - index at which element is to be set
    @param element - void* element to be set
    @return void
*/
void vector_set(vec_pointer v, int index, void* element);

/*
    Function pops void* element from the end of vector
    @param v - pointer to vector
    @return current size of vector
*/
int vector_pop(vec_pointer v);

/*
    Function clears vector's array
    @param v - pointer to vector
    @return void
*/
void vector_empty(vec_pointer v);

/*
    Function gets element of type int from vector at given index
    @param v - pointer to vector
    @param index - index of element to get
    @return int element at given index
*/
int v_get_int(vec_pointer v, int index);

/*
    Function gets element of type char from vector at given index
    @param v - pointer to vector
    @param index - index of element to get
    @return char element at given index
*/
char v_get_char(vec_pointer v, int index);

/*
    Function gets element of type vec_pointer from vector at given index
    @param v - pointer to vector
    @param index - index of element to get
    @return vec_pointer element at given index
*/
vec_pointer v_get_vec(vec_pointer v, int index);

/*
    Generic function to get element of given type from vector at given index
    @param type - type of element to get
    @param v - pointer to vector
    @param index - index of element to get
    @return element of given type at given index
*/
#define v_get(type, v, index) *((type*)(v->value) + index);

/*
    Additional utility functions
*/

/*
    Function creates vector from C-style string
    @param cstr - pointer to C-style string
    @param size - size of C-style string, -1 if null-terminated
    @return pointer to created vector
*/
vec_pointer vector_from_cstr(const char* cstr, ssize_t size);

/*
    Function prints vector's array separated by space
    @param v - pointer to vector
    @return void
*/
void vector_print(vec_pointer v);

#endif