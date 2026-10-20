#define _GNU_SOURCE
#define _XOPEN_SOURCE 700

#include "../includes/file.h"
#include "../includes/utils.h"

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

ssize_t read_bulk(int fd, char* buf, size_t count)
{
    size_t total_read = 0;
    while (total_read < count)
    {
        ssize_t curr_read = read(fd, buf + total_read, count - total_read);
        if (curr_read < 0)
            error("read error in read_bulk");
        if (curr_read == 0)
            break;
        total_read += curr_read;
    }
    return total_read;
}

ssize_t write_bulk(int fd, char* buf, size_t count)
{
    size_t total_write = 0;
    while (total_write < count)
    {
        ssize_t curr_written = write(fd, buf + total_write, count - total_write);
        if (curr_written < 0)
            error("write error in write_bulk");
        total_write += curr_written;
    }
    return total_write;
}

int copy_file(char* source, char* destination)
{
    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE];

    int src_fd = open(source, O_RDONLY);
    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0777);

    if (src_fd < 0)
        error("Error opening source file for reading");
    if (dest_fd < 0)
        error("Error opening destination file for writing");

    ssize_t current_read = 0;
    while (1)
    {
        current_read = read_bulk(src_fd, buf, BUF_SIZE);
        if (current_read == 0)
            break;
        write_bulk(dest_fd, buf, current_read);
    }

    try(close(src_fd));
    try(close(dest_fd));

    return 0;
}

int copy_directory(char* source, char* destination)
{
    DIR* dir = opendir(source);
    if (!dir)
        error("opendir");

    if (access(destination, F_OK) == -1)
        try(mkdir(destination, 0777));

    struct dirent* dp;
    struct stat st;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        char* src_path = join_paths(source, dp->d_name);
        char* dest_path = join_paths(destination, dp->d_name);

        try(lstat(src_path, &st));
        if (S_ISDIR(st.st_mode))
        {
            copy_directory(src_path, dest_path);
        }
        else
        {
            copy_file(src_path, dest_path);
        }

        free(src_path);
        free(dest_path);
    }

    closedir(dir);
    return 0;
}

int remove_directory(char* path)
{
    DIR* dir = opendir(path);
    if (!dir)
        error("opendir");

    struct dirent* dp;
    struct stat st;

    while ((dp = readdir(dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        char* curr_path = join_paths(path, dp->d_name);

        try(lstat(curr_path, &st));
        if (S_ISDIR(st.st_mode))
        {
            remove_directory(curr_path);
        }
        else
        {
            try(remove(curr_path));
        }

        free(curr_path);
    }

    closedir(dir);
    try(rmdir(path));
    return 0;
}

int ensure_directory_exists(char* path)
{
    struct stat st;
    if (stat(path, &st) == -1)
    {
        try(mkdir(path, 0777));
    }
    return 0;
}

int mkfakefifo(char* path)
{
    if (access(path, F_OK) == -1)
    {
        int res = open(path, O_CREAT | O_EXCL, 0666);
        if (res < 0)
            return res;
        try(close(res));
    }
    return 0;
}

// int move_file(char* source, char* destination){
//     return rename(source, destination);
// }

// int move_dir(char* source, char* destination){
//     return rename(source, destination);
// }

// int create_dir(char* destination){
//     return mkdir(destination, 0777);
// }

char* join_paths(char* path1, char* path2)
{
    if (path1[strlen(path1)] == '/')
    {
        path1[strlen(path1)] = 0;
    }

    if (path2[0] == '/')
    {
        const int p2len = strlen(path2);
        for (int i = 0; i < p2len - 1; i++)
            path2[i] = path2[i + 1];
        path2[p2len] = 0;
    }

    ssize_t buf_size = snprintf(NULL, 0, "%s/%s", path1, path2);
    char* result = (char*)malloc((buf_size + 1) * sizeof(char));
    snprintf(result, buf_size + 1, "%s/%s", path1, path2);

    return result;
}

char* get_relative_path(char* base_path, char* path)
{
    const int base_path_len = strlen(base_path);
    const int path_len = strlen(path);
    const int res_len = path_len - base_path_len - 1;  // base has /

    char* result = (char*)malloc((res_len + 1) * sizeof(char));

    strncpy(result, path + base_path_len + 1, res_len);
    result[res_len] = 0;
    return result;
}

bool is_subpath(char* base_path, char* path)
{
    size_t base_len = strlen(base_path);
    size_t path_len = strlen(path);

    if (path_len < base_len)
        return false;

    return strncmp(base_path, path, base_len) == 0;
}

int handle_create_file(char* base_path, char* source, char* destination)
{
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = copy_file(source, dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_create_dir(char* base_path, char* source, char* destination)
{
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = copy_directory(source, dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_delete_file(char* base_path, char* source, char* destination)
{
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = remove(dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_delete_dir(char* base_path, char* source, char* destination)
{
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = remove_directory(dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_modified_file(char* base_path, char* source, char* destination)
{
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    unlink(dest_path);
    int res = copy_file(source, dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_move(char* base_path, char* source, char* destination, char* new_source)
{
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    char* new_rel_path = get_relative_path(base_path, new_source);
    char* new_dest_path = join_paths(destination, new_rel_path);

    int res = rename(dest_path, new_dest_path);

    free(rel_path);
    free(dest_path);
    free(new_rel_path);
    free(new_dest_path);

    return res;
}

int recover_directory(char* destination, char* source)
{
    DIR* src_dir = opendir(source);
    if (!src_dir)
        error("opendir");

    ensure_directory_exists(destination);

    DIR* dest_dir = opendir(destination);
    if (!dest_dir)
        error("opendir");

    struct dirent* dp;
    struct stat st_src, st_dst;

    // remove non-existing files/dirs from destination
    while ((dp = readdir(dest_dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        char* curr_path = join_paths(source, dp->d_name);
        char* dest_path = join_paths(destination, dp->d_name);

        if (lstat(curr_path, &st_src) < 0)
        {
            try(lstat(dest_path, &st_dst));

            if (S_ISDIR(st_dst.st_mode))
            {
                remove_directory(dest_path);
            }
            else
            {
                try(remove(dest_path));
            }
        }

        free(curr_path);
        free(dest_path);
    }

    // sync rest of files
    while ((dp = readdir(src_dir)) != NULL)
    {
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        char* curr_path = join_paths(source, dp->d_name);
        char* dest_path = join_paths(destination, dp->d_name);

        try(lstat(curr_path, &st_src));
        if (lstat(dest_path, &st_dst) < 0)
        {
            if (S_ISDIR(st_src.st_mode))
            {
                copy_directory(curr_path, dest_path);
            }
            else
            {
                copy_file(curr_path, dest_path);
            }
        }
        else if (st_src.st_mtim.tv_sec > st_dst.st_mtim.tv_sec)
        {
            if (S_ISDIR(st_src.st_mode))
            {
                recover_directory(dest_path, curr_path);
            }
            else
            {
                try(remove(dest_path));
                copy_file(curr_path, dest_path);
            }
        }

        free(curr_path);
        free(dest_path);
    }

    try(closedir(src_dir));
    try(closedir(dest_dir));
    return 0;
}