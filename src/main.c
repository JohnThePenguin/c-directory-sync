#include <stdio.h>
#include <unistd.h>

#include "../includes/vector.h"
#include "../includes/structure.h"
#include "../includes/utils.h"
#include "../includes/sync.h"

pid_t setup_destination(v_string source, v_string destination, int fd){
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

int mainx() { 
  vec_pointer input = vector_create(sizeof(char));
  vec_pointer structure = create_structure();



  vector_delete(input);
  delete_structure(structure);
}
