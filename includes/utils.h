#ifndef UTILS_H
#define UTILS_H

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "vector.h"

enum program_error {
    ERR_SOURCE_NOT_FOUND,
    ERR_DESTINATION_NOT_FOUND,

    ERR_SOURCE_ALREADY_EXISTS,
    ERR_DESTINATION_ALREADY_EXISTS,

    ERR_SOURCE_DOES_NOT_EXIST,
    ERR_DESTINATION_DOES_NOT_EXIST,

    ERR_MEMORY_ALLOCATION,
};

#define error(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))

#define try(op) \
    if((op) < 0) error(#op)


size_t read_stdin(int stdin_fd, v_string buffer);

vec_pointer parse_line(v_string line);

vec_pointer handle_parsed_line(vec_pointer splitted);

bool validate_splitted_input(vec_pointer splitted);
    

#endif //UTILS_H