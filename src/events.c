#include "../includes/events.h"
#include "../includes/utils.h"

void* read_event(int fd, size_t event_size){
    void* event = malloc(event_size);
    if(!event) error("Malloc error in read_event");

    ssize_t total_read = 0;
    while(total_read < (ssize_t)event_size){
        ssize_t curr_read = read(fd, event + total_read, event_size - total_read);
        if(curr_read < 0) {
            error("Read error in read_event, could not read full event");
        }
        total_read += curr_read;
    }

    return event;
}

int write_event(int fd, void* event, size_t event_size){
    ssize_t total_written = 0;
    while(total_written < (ssize_t)event_size){
        ssize_t curr_written = write(fd, event + total_written, event_size - total_written);
        if(curr_written < 0) return -1;
        total_written += curr_written;
    }
    return 0;
}

controller_event_pointer read_controller_event(int fd){
    return (controller_event_pointer)read_event(fd, sizeof(struct controller_event));
}

sync_event_pointer read_sync_event(int fd){
    return (sync_event_pointer)read_event(fd, sizeof(struct sync_event));
}