#include "../includes/vector.h"
#include "../includes/sync.h"
#include "../includes/utils.h"
#include "../includes/events.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void handle_sync_event(sync_event_pointer event, const char* source, const char* destination){
    switch (event->type){
        case ET_FILE_CREATED:
            break;
        case ET_FILE_DELETED:
            break;
        case ET_FILE_MODIFIED:
            break;
        case ET_DIR_CREATED:
            break;
        case ET_DIR_DELETED:
            break;
        default:
            break;
    }
}

void run_sync(const char* source, const char* destinations, char* fifo_path){
    printf("Starting sync from %s to %s using fifo %s\n",
        source, destinations, fifo_path);

    // Copy source to destination 

    // Enter loop reading events from fifo_path and syncing files
    int fifo_fd = open(fifo_path, O_RDONLY);
    if(fifo_fd < 0) error("Error opening fifo in run_sync, stopping sync process");

    while(1){
        sync_event_pointer event = read_sync_event(fifo_fd);
        printf("Sync process received event %d on path %s\n",
            event->type, event->path);

        handle_sync_event(event, source, destinations);

        free(event);
    }

    // Cleanup
    try(close(fifo_fd));
}

void run_recover(const char* source, const char* destinations){
    printf("Starting recover from %s to %s\n",
        source, destinations);

}