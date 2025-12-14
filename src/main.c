#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "../includes/vector.h"
#include "../includes/structure.h"
#include "../includes/utils.h"
#include "../includes/sync.h"

pid_t spawn_sync(v_string source, v_string destination, int fd){
    pid_t f = fork();
    if(f < 0){
        error("Fork error");
    }
    if(f == 0){
        run_sync(source, destination, fd);

        if(close(fd) < 0) error("Close pipe error in child");
        exit(EXIT_SUCCESS);
    } else {
        return f;
    }
}

int setup_destination(vec_pointer structure, v_string source, v_string destination, int fd){
    dest_t dest = get_destination(structure, source, destination);
    if(!dest.not_found){
        return ERR_DESTINATION_ALREADY_EXISTS;
    }

    source_t src = get_source(structure, source);
    int pipe_fd[2] = {src.pipe_rd_fd, src.pipe_wr_fd};

    if(src.not_found){
        if(pipe(pipe_fd) < 0) error("Pipe creation error");
        add_source(structure, source, pipe_fd[0], pipe_fd[1]);
    }

    pid_t pid = spawn_sync(source, destination, pipe_fd[0]);

    add_destination(structure, source, destination, pid);

    return 0;
}

int handle_input(vec_pointer structure, v_string input){
    printf("Handling input: %s\n", input->value);

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
            printf("Read %zu bytes from stdin\n", bytes_read);

            handle_input(structure, input_buffer);
        }
    }

    close(inotify_fd);

    vector_delete(input_buffer);
    unlink(tempfilename);
    delete_structure(structure);

    return EXIT_SUCCESS;
}
