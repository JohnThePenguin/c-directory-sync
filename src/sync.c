#include "../includes/vector.h"
#include "../includes/sync.h"

#include <stdio.h>

void run_sync(const char* source, const char* destinations, char* fifo_path){
    printf("Starting sync from %s to %s using fifo %s\n",
        source, destinations, fifo_path);

    // Copy source to destination 

    // Enter loop reading events from fifo_path and syncing files
}

void run_recover(const char* source, const char* destinations){
    printf("Starting recover from %s to %s\n",
        source, destinations);

}