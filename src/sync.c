#define _GNU_SOURCE
#define _XOPEN_SOURCE 700

#include "../includes/events.h"
#include "../includes/file.h"
#include "../includes/sync.h"
#include "../includes/utils.h"
#include "../includes/vector.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/select.h>
#include <unistd.h>

void handle_sync_event(sync_event_pointer event, char* source, char* destination)
{
    switch (event->type)
    {
        case ET_FILE_CREATED:
            handle_create_file(source, event->path, destination);
            break;
        case ET_FILE_DELETED:
            handle_delete_file(source, event->path, destination);
            break;
        case ET_FILE_MODIFIED:
            handle_modified_file(source, event->path, destination);
            break;
        case ET_DIR_CREATED:
            handle_create_dir(source, event->path, destination);
            break;
        case ET_DIR_DELETED:
            handle_delete_dir(source, event->path, destination);
            break;
        default:
            break;
    }
}

void run_sync(const char* source, const char* destinations, char* fifo_path)
{
    copy_directory((char*)source, (char*)destinations);

    int fifo_fd = open(fifo_path, O_RDONLY | O_NONBLOCK);
    if (fifo_fd < 0)
        error("Error opening fifo in run_sync, stopping sync process");

    try(lseek(fifo_fd, 0, SEEK_END));

    // int signal_fd = setup_signal_fd();

    fd_set read_fd;
    int max_fd = fifo_fd;  // (fifo_fd > signal_fd) ? fifo_fd : signal_fd;

    while (1)
    {
        FD_ZERO(&read_fd);
        FD_SET(fifo_fd, &read_fd);
        // FD_SET(signal_fd, &read_fd);
        int activity = select(max_fd + 1, &read_fd, NULL, NULL, NULL);
        if (activity < 0)
            error("select error in controller_loop");

        // if(FD_ISSET(signal_fd, &read_fd)){
        //     break;
        // }
        if (!FD_ISSET(fifo_fd, &read_fd))
        {
            continue;
        }

        sync_event_pointer event = read_sync_event(fifo_fd);

        handle_sync_event(event, (char*)source, (char*)destinations);

        free(event);
    }

    printf("Sync process exiting\n");

    // Cleanup
    try(close(fifo_fd));
    // try(close(signal_fd));
}

void run_recover(const char* source, const char* destinations) {}