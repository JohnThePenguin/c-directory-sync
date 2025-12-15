
// Original source: https://gitlab.com/SaQQ/sop1/-/blob/master/05_events/watch_tree.c

#include <dirent.h>
#include <limits.h>
#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../includes/structure.h"
#include "../includes/utils.h"
#include "../includes/events.h"
#include "../includes/file.h"

#define MAX_WATCHES 8192
#define EVENT_BUF_LEN (64 * (sizeof(struct inotify_event) + NAME_MAX + 1))

struct Watch
{
    int wd;      // key
    char* path;  // value
};

typedef struct Watch* watch_t;

struct WatchMap
{
    struct Watch watch_map[MAX_WATCHES];
    int watch_count;
    int inotify_fd;
};

typedef struct WatchMap* watch_map_t;

void add_to_map(watch_map_t map, int wd, const char* path);

watch_t find_watch(watch_map_t map, int wd);

void remove_from_map(watch_map_t map, int wd);

void add_watch_recursive(int notify_fd, watch_map_t map, const char* base_path);

void update_watch_paths(watch_map_t map, const char* old_path, const char* new_path);

watch_map_t setup_inotify();

int delete_inotify(watch_map_t map);

void notify_sources(vec_pointer structure, enum sync_event_type et, char* path);

void notify_sources_create(vec_pointer structure, char* path, bool is_dir);

void notify_sources_delete(vec_pointer structure, char* path, bool is_dir);

void notify_sources_modify(vec_pointer structure, char* path);

void handle_inotify_ready(vec_pointer structure, watch_map_t map);
