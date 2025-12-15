#ifndef EVENTS_H
#define EVENTS_H

#include <stddef.h>

#define PATH_SIZE 2048

enum controller_event_type {
    ET_SYNC_START = 0,
    ET_SYNC_STOP = 1,
    ET_RESTORE = 2
};

struct controller_event {
    int key; // Unique key for source-destination pair, stored in structure
    int source_key; // Key assigned to source path for pipe
    enum controller_event_type type;
    char source[PATH_SIZE];
    char destination[PATH_SIZE];
};

typedef struct controller_event* controller_event_pointer;


enum sync_event_type {
    ET_FILE_CREATED = 0,
    ET_FILE_DELETED = 1,
    ET_FILE_MODIFIED = 2,
    // ET_FILE_MOVED = 3,
    ET_DIR_CREATED = 4,
    ET_DIR_DELETED = 5,
    // ET_DIR_MOVED = 6
};

struct sync_event {
    enum sync_event_type type;
    char path[PATH_SIZE];
};

typedef struct sync_event* sync_event_pointer;


void* read_event(int fd, size_t event_size);

int write_event(int fd, void* event, size_t event_size);

controller_event_pointer read_controller_event(int fd);

sync_event_pointer read_sync_event(int fd);

#endif // EVENTS_H