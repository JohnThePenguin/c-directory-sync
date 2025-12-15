#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>

#define CONTROLLER_MAX_FORKS (int)(1e6+7)


/*
    @param key - unique key for source
    @param pid - pid of the main process
    @return path to fifo (newly allocated string)
*/
char* get_fifo_path(int key, pid_t pid);

/*
    @param key - unique key for source
    @param pid - pid of the main process
    @return fd of opened pipe
*/
int open_pipe_fd(int key, pid_t pid);

pid_t create_controller(int pipe_fd[2], char* base_path);

void controller_loop(int pipe_fd, char* base_path);


#endif // CONTROLLER_H