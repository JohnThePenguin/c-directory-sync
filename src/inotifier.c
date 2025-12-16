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
#include "../includes/inotifier.h"

void add_to_map(watch_map_t map, int wd, const char* path)
{
    if (map->watch_count >= MAX_WATCHES)
    {
        error("max watches exceeded");
    }
    map->watch_map[map->watch_count].wd = wd;
    map->watch_map[map->watch_count].path = strdup(path);  // Must copy the path!
    map->watch_count++;
}

watch_t find_watch(watch_map_t map, int wd)
{
    for (int i = 0; i < map->watch_count; i++)
    {
        if (map->watch_map[i].wd == wd)
        {
            return &map->watch_map[i];
        }
    }
    return NULL;
}

void remove_from_map(watch_map_t map, int wd)
{
    for (int i = 0; i < map->watch_count; i++)
    {
        if (map->watch_map[i].wd == wd)
        {
            free(map->watch_map[i].path);
            map->watch_map[i] = map->watch_map[map->watch_count - 1];
            map->watch_count--;
            return;
        }
    }
}

void add_watch_recursive(int notify_fd, watch_map_t map, const char* base_path)
{
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO;

    int wd = inotify_add_watch(notify_fd, base_path, mask);
    if (wd < 0)
        error("inotify_add_watch");

    add_to_map(map, wd, base_path);

    DIR* dir = opendir(base_path);
    if (!dir)
        error("opendir");

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char full_path[PATH_MAX];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_path, entry->d_name);

        struct stat st;
        if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode))
        {
            add_watch_recursive(notify_fd, map, full_path);
        }
    }

    closedir(dir);
}

void update_watch_paths(watch_map_t map, const char* old_path, const char* new_path)
{
    size_t old_len = strlen(old_path);

    for (int i = 0; i < map->watch_count; i++)
    {
        if (strncmp(map->watch_map[i].path, old_path, old_len) == 0 &&
            (map->watch_map[i].path[old_len] == '/' || map->watch_map[i].path[old_len] == '\0'))
        {
            char new_full_path[PATH_MAX];
            snprintf(new_full_path, sizeof(new_full_path), "%s%s", new_path, map->watch_map[i].path + old_len);

            free(map->watch_map[i].path);
            map->watch_map[i].path = strdup(new_full_path);
        }
    }
}

watch_map_t setup_inotify()
{
    int inotify_fd = inotify_init1(IN_NONBLOCK);
    if (inotify_fd < 0)
    {
        error("inotify_init");
        exit(EXIT_FAILURE);
    }

    watch_map_t map = (watch_map_t)malloc(sizeof(struct WatchMap));
    map->inotify_fd = inotify_fd;
    map->watch_count = 0;
    return map;
}

int delete_inotify(watch_map_t map)
{
    try(close(map->inotify_fd));
    free(map);
    return 0;
}

void notify_sources(vec_pointer structure, enum sync_event_type et, char* path){
    sync_event_pointer event = malloc(sizeof(struct sync_event));
    event->type = et;
    strcpy(event->path, path);

    for(int i = 0; i < structure->size; i++){
        source_t source = v_get_source(structure, i);
        if(is_subpath(source.source->value, path)){
            // printf("Notifying source %s about event %d on path %s\n",
                // source.source->value, et, path);

            ssize_t res = write_event(source.pipe_wr_fd, event, sizeof(struct sync_event));
            if(res < 0) printf("Error writing to source pipe (fd = %ld)\n", source.pipe_wr_fd);
        }
    }
    free(event);
}

void notify_sources_create(vec_pointer structure, char* path, bool is_dir){
    enum sync_event_type et = is_dir ? ET_DIR_CREATED : ET_FILE_CREATED;
    notify_sources(structure, et, path);
}

void notify_sources_delete(vec_pointer structure, char* path, bool is_dir){
    enum sync_event_type et = is_dir ? ET_DIR_DELETED : ET_FILE_DELETED;
    notify_sources(structure, et, path);
}

void notify_sources_modify(vec_pointer structure, char* path){
    notify_sources(structure, ET_FILE_MODIFIED, path);
}

void handle_inotify_ready(vec_pointer structure, watch_map_t map)
{
    uint32_t pending_cookie = 0;
    char pending_move_path[PATH_MAX] = "";

    char buffer[EVENT_BUF_LEN];
    ssize_t len = read(map->inotify_fd, buffer, EVENT_BUF_LEN);
    if (len < 0)
        error("inotify read");

    ssize_t i = 0;
    while (i < len)
    {
        struct inotify_event* event = (struct inotify_event*)&buffer[i];

        // printf("[i=%ld] [ev=0x%08x] [wd=%d] ", i, event->mask, event->wd);

        struct Watch* watch = find_watch(map, event->wd);
        bool is_dir = event->mask & IN_ISDIR;

        char event_path[PATH_MAX] = "";
        if (watch && event->len > 0)
        {
            snprintf(event_path, sizeof(event_path), "%s/%s", watch->path, event->name);
        }
        else if (watch)
        {
            strncpy(event_path, watch->path, sizeof(event_path));
        }

        if (event->mask & IN_IGNORED)
        {
            remove_from_map(map, event->wd);
            i += sizeof(struct inotify_event) + event->len;
            continue;
        }

        if (event->mask & IN_CREATE)
        {
            notify_sources_create(structure, event_path, is_dir);

            if( event->mask & IN_ISDIR ) 
                add_watch_recursive(map->inotify_fd, map, event_path);
        }
        if (event->mask & IN_DELETE)
        {
            notify_sources_delete(structure, event_path, is_dir);
        }
        else if (event->mask & IN_MOVED_FROM)
        {
            pending_cookie = event->cookie;
            strncpy(pending_move_path, event_path, sizeof(pending_move_path));

            notify_sources_delete(structure, event_path, is_dir);
        }
        else if (event->mask & IN_MOVED_TO)
        {
            if(is_dir){
                if (event->cookie == pending_cookie && pending_cookie != 0)
                {
                    update_watch_paths(map, pending_move_path, event_path);
                    pending_cookie = 0;
                    pending_move_path[0] = '\0';
                }
                else
                {
                    add_watch_recursive(map->inotify_fd, map, event_path);
                }
            }

            notify_sources_create(structure, event_path, is_dir);
        }
        else if (event->mask & IN_MODIFY)
        {
            if(!is_dir)
                notify_sources_modify(structure, event_path);
        }

        i += sizeof(struct inotify_event) + event->len;
    }

}
