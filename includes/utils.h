#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

enum program_error {
    SOURCE_NOT_FOUND,
    DESTINATION_NOT_FOUND,

    SOURCE_ALREADY_EXISTS,
    DESTINATION_ALREADY_EXISTS,

    SOURCE_DOES_NOT_EXIST,
    DESTINATION_DOES_NOT_EXIST,

    MEMORY_ALLOCATION_ERROR,
};

#define error(source) \
    (fprintf(stderr, "%s:%d\n", __FILE__, __LINE__), perror(source), kill(0, SIGKILL), exit(EXIT_FAILURE))








#endif //UTILS_H