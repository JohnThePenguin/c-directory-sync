#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/select.h>

#include "../includes/vector.h"
#include "../includes/structure.h"
#include "../includes/utils.h"
#include "../includes/sync.h"
#include "../includes/events.h"
#include "../includes/controller.h"
#include "../includes/inotifier.h"

struct s_main {
    vec_pointer structure;
    watch_map_t inotify_map;
    int controller_fd;
    int* max_dest_key;
};


int setup_destination(struct s_main dt, v_string source, v_string destination){
    dest_t dest = get_destination(dt.structure, source, destination);
    if(!dest.not_found){
        return ERR_DESTINATION_ALREADY_EXISTS;
    }

    source_t src = get_source(dt.structure, source);

    int new_key = dt.structure->size + 1;
    if(src.not_found){
        int pipe_fd = open_pipe_fd(new_key, getpid());

        printf("In setup_destination: Adding source %s with key %d\n", source->value, new_key);        

        add_source(dt.structure, source, new_key, pipe_fd);
        // Duplicate should be ok
        add_watch_recursive(dt.inotify_map->inotify_fd, dt.inotify_map, source->value);
    }

    *(dt.max_dest_key) += 1;
    add_destination(dt.structure, source, destination, *(dt.max_dest_key));

    controller_event_pointer event = malloc(sizeof(struct controller_event));
    event->key = *(dt.max_dest_key);
    event->source_key = src.not_found ? new_key : src.key;
    event->type = ET_SYNC_START;
    strcpy(event->source, source->value);
    strcpy(event->destination, destination->value);

    write_event(dt.controller_fd, event, sizeof(struct controller_event));
    free(event);

    return 0;
}

int destruct_destination(struct s_main dt, v_string source, v_string destination){
    dest_t dest = get_destination(dt.structure, source, destination);
    if(dest.not_found){
        printf("Destination %s not found for source %s\n", destination->value, source->value);
        return ERR_DESTINATION_NOT_FOUND;
    }

    controller_event_pointer event = malloc(sizeof(struct controller_event));
    event->key = dest.key;
    event->source_key = -1;      // Not needed for stop
    event->type = ET_SYNC_STOP;
    strcpy(event->source, source->value);
    strcpy(event->destination, destination->value);

    printf("Sending stop event for source %s and destination %s\n", source->value, destination->value);
    write_event(dt.controller_fd, event, sizeof(struct controller_event));
    free(event);

    remove_destination(dt.structure, source, destination);

    return 0;
}

void setup_recover(struct s_main dt, v_string source, v_string destination){
    controller_event_pointer event = malloc(sizeof(struct controller_event));
    event->key = -1;            // Not needed for recover
    event->source_key = -1;     // Not needed for recover
    event->type = ET_RESTORE;
    strcpy(event->source, source->value);
    strcpy(event->destination, destination->value);

    write_event(dt.controller_fd, event, sizeof(struct controller_event));
    free(event);
}

int handle_input(struct s_main dt, v_string input){
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

        if(!strcmp(command->value, "add")){
            setup_destination(dt, source, v_get_vec(parsed, i));
        }
        if(!strcmp(command->value, "remove")){
            destruct_destination(dt, source, v_get_vec(parsed, i));
        }
        if(!strcmp(command->value, "recover")){
            setup_recover(dt, source, v_get_vec(parsed, i));
        }

        vector_delete(v_get_vec(parsed, i));
    }

    if(!strcmp(command->value, "list")){
        printf_structure(dt.structure);
    }
    if(!strcmp(command->value, "exit")){
        printf("Exiting...\n");
        exit(EXIT_SUCCESS);
    }

    vector_delete(parsed);

    return 0;
}

int handle_inotify_event(struct s_main dt){
    handle_inotify_ready(dt.structure, dt.inotify_map);
    return 0;
}


int main() { 
    vec_pointer structure = create_structure();
    v_string input_buffer = vector_create(sizeof(char));
    vector_set_size(input_buffer, 128);

    int max_dest_key = 1;

    watch_map_t inotify_map = setup_inotify();
    int inotify_fd = inotify_map->inotify_fd;
    int stdin_fd = STDIN_FILENO;

    int controller_pipe[2];
    try(pipe(controller_pipe));
    pid_t controller_pid = create_controller(controller_pipe, ".");
    controller_pipe[0] = -1; // Closed during controller creation
    

    fcntl(stdin_fd, F_SETFL, O_NONBLOCK);
    int max_fd = (inotify_fd > stdin_fd) ? inotify_fd : stdin_fd;
    fd_set read_fd;

    struct s_main main_data = {
        structure,
        inotify_map,
        controller_pipe[1],
        &max_dest_key
    };

    // Main loop
    while(1){
        FD_ZERO(&read_fd);
        FD_SET(inotify_fd, &read_fd);
        FD_SET(stdin_fd, &read_fd);

        fflush(stdout);
        int activity = select(max_fd + 1, &read_fd, NULL, NULL, NULL); 

        if(activity < 0) error("Select error");

        if(FD_ISSET(inotify_fd, &read_fd)){
            handle_inotify_event(main_data);
        }

        if(FD_ISSET(stdin_fd, &read_fd)){
            size_t bytes_read = read_stdin(stdin_fd, input_buffer);
            // printf("Read %zu bytes from stdin\n", bytes_read);

            handle_input(main_data, input_buffer);
        }
    }

    close(inotify_fd);

    vector_delete(input_buffer);

    // Close all pipes and controller
    delete_structure(structure);
    delete_inotify(inotify_map);

    return EXIT_SUCCESS;
}
