#ifndef SYNC_H
#define SYNC_H

#include "vector.h"

void run_sync(const char* source, const char* destinations, char* fifo_path);

void run_recover(const char* source, const char* destinations);

#endif // SYNC_H