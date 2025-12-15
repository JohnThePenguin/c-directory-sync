#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "../includes/vector.h"
#include "../includes/structure.h"
#include "../includes/utils.h"
#include "../includes/sync.h"
#include "../includes/events.h"
#include "../includes/controller.h"

int setup_destination(vec_pointer structure, v_string source, v_string destination, int controller_fd){
    dest_t dest = get_destination(structure, source, destination);
    if(!dest.not_found){
        return ERR_DESTINATION_ALREADY_EXISTS;
    }

    source_t src = get_source(structure, source);

    if(src.not_found){
        add_source(structure, source, -1);
    }

    int destination_key = -2; // Generate unique key for destination
    add_destination(structure, source, destination, destination_key);

    controller_event_pointer event = malloc(sizeof(struct controller_event));
    event->key = destination_key;
    event->source_key = src.key;
    event->type = ET_SYNC_START;
    strcpy(event->source, source->value);
    strcpy(event->destination, destination->value);

    write_event(controller_fd, event, sizeof(struct controller_event));
    free(event);

    return 0;
}

int destruct_destination(vec_pointer structure, v_string source, v_string destination, int controller_fd){
    dest_t dest = get_destination(structure, source, destination);
    if(dest.not_found){
        return ERR_DESTINATION_NOT_FOUND;
    }

    controller_event_pointer event = malloc(sizeof(struct controller_event));
    event->key = dest.key;
    event->source_key = -1; // Not needed for stop
    event->type = ET_SYNC_STOP;
    strcpy(event->source, source->value);
    strcpy(event->destination, destination->value);

    write_event(controller_fd, event, sizeof(struct controller_event));
    free(event);

    remove_destination(structure, source, destination);

    return 0;
}

void setup_recover(vec_pointer structure, v_string source, v_string destination, int controller_fd){
    controller_event_pointer event = malloc(sizeof(struct controller_event));
    event->key = -1; // Not needed for recover
    event->source_key = -1; // Not needed for recover
    event->type = ET_RESTORE;
    strcpy(event->source, source->value);
    strcpy(event->destination, destination->value);

    write_event(controller_fd, event, sizeof(struct controller_event));
    free(event);
}

int handle_input(vec_pointer structure, v_string input){
    // printf("Handling input: %s\n", input->value);
    if(input->size <= 2) return -1;

    vec_pointer splitted = parse_line(input);

    vec_pointer parsed = handle_parsed_line(splitted);
    if(parsed == NULL || !validate_splitted_input(parsed)){
        for(int i = 0; i < splitted->size; i++)
            vector_delete(v_get_vec(splitted, i));
        vector_delete(splitted);
        return -1;
    }

    v_string command = v_get_vec(splitted, 0);
    v_string source = v_get_vec(splitted, 1);

    for(int i = 2; i < parsed->size; i++){
        printf("Running %s with source %s and destination %s\n",
            command->value, source->value,
            v_get_vec(parsed, i)->value);


        vector_delete(v_get_vec(parsed, i));
    }
    vector_delete(parsed);

    return 0;

    if(!strcmp(command->value, "add")){

    } else if(!strcmp(command->value, "remove")){

    } else if(!strcmp(command->value, "list")){
    
    } else if(!strcmp(command->value, "recover")){

    } else if(!strcmp(command->value, "exit")){

    } else {

    }

    return 0;
}

int handle_inotify_event(vec_pointer structure, vec_pointer event){
    // Read and handle inotify events
    return 0;
}


int main() { 
    vec_pointer structure = create_structure();
    v_string input_buffer = vector_create(sizeof(char));
    vector_set_size(input_buffer, 128);

    char tempfilename[] = "/tmp/inputXXXXXX";
    int inotify_fd = mkstemp(tempfilename); // Initialize inotify mockup
    int stdin_fd = STDIN_FILENO;

    long controller_pipe[2];
    try(pipe(controller_pipe));
    pid_t controller_pid = create_controller(controller_pipe, ".");

    fcntl(stdin_fd, F_SETFL, O_NONBLOCK);
    int max_fd = (inotify_fd > stdin_fd) ? inotify_fd : stdin_fd;
    fd_set read_fd;

    // Main loop
    while(1){
        FD_ZERO(&read_fd);
        FD_SET(inotify_fd, &read_fd);
        FD_SET(stdin_fd, &read_fd);

        fflush(stdout);
        int activity = select(max_fd + 1, &read_fd, NULL, NULL, NULL); 

        if(activity < 0) error("Select error");

        if(FD_ISSET(inotify_fd, &read_fd)){
            handle_inotify_event(structure, NULL); // Mock inotify handling
        }

        if(FD_ISSET(stdin_fd, &read_fd)){
            size_t bytes_read = read_stdin(stdin_fd, input_buffer);
            // printf("Read %zu bytes from stdin\n", bytes_read);

            handle_input(structure, input_buffer);
        }
    }

    close(inotify_fd);

    vector_delete(input_buffer);
    unlink(tempfilename);
    delete_structure(structure);

    return EXIT_SUCCESS;
}
