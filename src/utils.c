#include <unistd.h>
#include <errno.h>

#include "../includes/utils.h"
#include "../includes/vector.h"

size_t read_stdin(int stdin_fd, v_string buffer){
    ssize_t already_read = 0;
    char null_char = '\0';

    while(1){
        if(already_read >= buffer->allocated - 1){
            vector_set_size(buffer, buffer->allocated * 2);
        }

        // Wrap into TEMP_FAIL macro
        ssize_t bytes_read = read(stdin_fd, 
            buffer->value + already_read, 
            buffer->allocated - already_read - 1
        );

        if(bytes_read < 0){
            if(errno == EAGAIN || errno == EWOULDBLOCK) break;
            error("Read error from stdin");
        } else if(bytes_read == 0){
            break;
        } else {
            already_read += bytes_read;
        }
    }

    vector_set(buffer, already_read, &null_char);
    return already_read;
}