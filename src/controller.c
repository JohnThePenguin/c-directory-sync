#include "../includes/controller.h"
#include "../includes/events.h"
#include "../includes/file.h"
#include "../includes/sync.h"
#include "../includes/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

pid_t create_controller(int pipe_fd[2], char* base_path)
{
    pid_t f = fork();
    if (f < 0)
    {
        perror("Fork error in create_controller");
        exit(EXIT_FAILURE);
    }
    if (f == 0)
    {
        controller_loop(pipe_fd[0], base_path);

        // try(close(pipe_fd[0])); closed in controller_loop
        try(close(pipe_fd[1]));
        exit(EXIT_SUCCESS);
    }
    else
    {
        try(close(pipe_fd[0]));
        return f;
    }
}

char* get_fifo_path(int key, pid_t pid)
{
    size_t size = 32;  // 13 + 7 + 7 + margin;
    char* fifo_name = malloc(size * sizeof(char));
    snprintf(fifo_name, size, "/tmp/dirsync/%d/%d", pid, key);
    return fifo_name;
}

int open_pipe_fd(int key, pid_t pid)
{
    char* pipe_path = get_fifo_path(key, pid);
    // printf("Opening fifo pipe at %s for key %d\n", pipe_path, key);

    try(mkfakefifo(pipe_path));

    int fd = open(pipe_path, O_RDWR);

    if (fd < 0)
    {
        printf("Error opening fifo pipe for key %d\n", key);
    }

    free(pipe_path);
    return fd;
}

void start_sync_process(int key, int source_key, char* source, char* destination, pid_t* set_pid, int main_pipe_fd,
                        int signal_fd)
{
    char* fifo_path = get_fifo_path(source_key, getppid());

    // Fifos are per source, so it can exist already
    mkfakefifo(fifo_path);

    pid_t f = fork();
    if (f < 0)
        error("Fork error in start_sync_process");

    if (f == 0)
    {
        try(close(main_pipe_fd));
        try(close(signal_fd));
        set_clean_mask();

        run_sync(source, destination, fifo_path);

        free(fifo_path);
        exit(EXIT_SUCCESS);
    }
    else
    {
        *set_pid = f;
    }

    free(fifo_path);
}

int stop_sync_process(pid_t pid)
{
    try(kill(pid, SIGTERM));
    int status;
    try(waitpid(pid, &status, 0));

    // printf("Controller: Stopped and waited process %d\n", pid);
    return status;
}

void start_recover_process(int key, char* source, char* destination, pid_t* set_pid, int main_pipe_fd)
{
    pid_t f = fork();
    if (f < 0)
        error("Fork error in start_recover_process");

    if (f == 0)
    {
        try(close(main_pipe_fd));

        run_recover(source, destination);

        exit(EXIT_SUCCESS);
    }
    else
    {
        *set_pid = f;
    }
}

void controller_loop(int pipe_fd, char* base_path)
{
    pid_t fork_pids[CONTROLLER_MAX_FORKS];
    int fork_fds[CONTROLLER_MAX_FORKS];
    int max_dest_key = 0;

    memset(fork_pids, 0, sizeof(fork_pids));
    memset(fork_fds, 0, sizeof(fork_fds));

    char* fifo_dir = malloc(32 * sizeof(char));
    snprintf(fifo_dir, 32, "/tmp/dirsync/%d", getppid());
    ensure_directory_exists("/tmp/dirsync");
    ensure_directory_exists(fifo_dir);

    int signal_fd = setup_signal_fd();
    fcntl(pipe_fd, F_SETFL, O_NONBLOCK);

    fd_set read_fd;
    int max_fd = (pipe_fd > signal_fd) ? pipe_fd : signal_fd;

    while (1)
    {
        FD_ZERO(&read_fd);
        FD_SET(pipe_fd, &read_fd);
        FD_SET(signal_fd, &read_fd);
        int activity = select(max_fd + 1, &read_fd, NULL, NULL, NULL);
        if (activity < 0)
            error("select error in controller_loop");

        if (FD_ISSET(signal_fd, &read_fd))
        {
            break;
        }
        if (!FD_ISSET(pipe_fd, &read_fd))
        {
            continue;
        }

        controller_event_pointer event = read_controller_event(pipe_fd);
        if (event->key > max_dest_key)
            max_dest_key = event->key;

        switch (event->type)
        {
            case ET_SYNC_START:
                // printf("Controller: Start sync from %s to %s (source-key=%d)\n", event->source, event->destination,
                // event->source_key);
                start_sync_process(event->key, event->source_key, event->source, event->destination,
                                   &fork_pids[event->key], pipe_fd, signal_fd);
                break;
            case ET_SYNC_STOP:
                // printf("Controller: Stop sync from %s to %s\n", event->source, event->destination);
                stop_sync_process(fork_pids[event->key]);
                fork_pids[event->key] = 0;
                break;
            case ET_RESTORE:
                // printf("Controller: Restore from %s to %s\n", event->source, event->destination);
                start_recover_process(event->key, event->source, event->destination, &fork_pids[event->key], pipe_fd);
                break;
            default:
                // printf("Controller: Unknown event type %d\n", event->type);
                break;
        }

        free(event);
    }

    printf("Controller: exiting\n");

    for (int i = 0; i <= max_dest_key; i++)
    {
        if (fork_pids[i] != 0)
        {
            stop_sync_process(fork_pids[i]);
        }
    }

    close(signal_fd);
    close(pipe_fd);

    unlink(fifo_dir);
    free(fifo_dir);
}
