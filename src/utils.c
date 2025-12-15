#include <unistd.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include "../includes/utils.h"
#include "../includes/vector.h"
#include "../includes/file.h"

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
    buffer->size = already_read + 1;
    return already_read;
}

int find_char(v_string str, char c, int start_index){
    for(int i = start_index; i < str->size; i++){
        if(v_get_char(str, i) == c){
            return i;
        }
    }
    return -1;
}

vec_pointer parse_line(v_string line){
    vec_pointer result = vector_create(sizeof(v_string));
    int i = 0;
    char null_char = '\0';
   
    while(i < line->size - 2){ // enter and null char
        while(i < line->size && v_get_char(line, i) == ' ') i++;
        if(i >= line->size - 2) break;

        bool in_quotes = v_get_char(line, i) == '"';

        if(in_quotes) i++;
        int start = i;
        int end = find_char(line, in_quotes ? '"' : ' ', i);

        if(end == -1) end = line->size - 2; // until enter

        vec_pointer segment = vector_create(sizeof(char));
        vector_set_size(segment, end - start + 1);
        memcpy(segment->value, line->value + start, end - start);

        vector_set(segment, end - start, &null_char);

        vector_push(result, &segment);
        i += (in_quotes ? 1 : 0) + (end - start) + 1;
    }

    return result;
}

vec_pointer handle_parsed_line(vec_pointer splitted){
    for(int i = 1; i < splitted->size; i++){
        v_string* path = ((v_string*)(splitted->value) + i);

        if(i >= 2) { 
            ensure_directory_exists((*path)->value);
        }

        char* cstr_path = realpath((*path)->value, NULL);

        if(cstr_path == NULL){
            printf("Path %s not found\n", (*path)->value);
            return NULL;
        }

        v_string abs_path = vector_from_cstr(cstr_path, -1);
        vector_rewrite(path, &abs_path);

        free(cstr_path);
    }

    return splitted;
}

bool validate_splitted_input(vec_pointer splitted){
    // Check for:
    // - valid command
    // - correct paths - existance, type, emptiness
    // - destination is not inside source 
    if(splitted->size == 0) return false;

    v_string command = v_get_vec(splitted, 0);

    if(!strcmp(command->value, "add") || 
        !strcmp(command->value, "remove") || 
        !strcmp(command->value, "recover")){

        if(splitted->size < 3){
            printf("Command %s requires at least 2 arguments\n", (char*)command->value);
            return false;
        }

        return true;
    }

    if(!strcmp(command->value, "list") || !strcmp(command->value, "exit")){
        if(splitted->size != 1){
            printf("Command list does not take any arguments\n");
            return false;
        }

        return true;
    }
   
    printf("Unknown command: %s\n", (char*)command->value);
    return false;
}