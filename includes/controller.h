#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <sys/types.h>

#define CONTROLLER_MAX_FORKS (int)(1e6+7)

pid_t create_controller(long pipe_fd[2], char* base_path);


#endif // CONTROLLER_H