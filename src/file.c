#include "../includes/file.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>


int copy_file(char* source, char* destination){
    const int BUF_SIZE = 4096;
    char buf[BUF_SIZE];

    int src_fd = open(source, O_RDONLY);
    int dest_fd = open(destination, O_WRONLY | O_CREAT | O_TRUNC | O_EXCL, 0777);

    ssize_t all_read = 0;
    ssize_t curr_read = 0;
    ssize_t all_write = 0;

    curr_read = read(src_fd, buf, BUF_SIZE);
    while(curr_read > 0){
        ssize_t curr_write = 0;
        ssize_t total_write = 0;

        while(total_write <= curr_read){
            curr_write = 
                write(dest_fd, buf+curr_read, curr_read - total_write);

            total_write += curr_read;
        }
        all_write += total_write;
        curr_read = read(src_fd, buf, BUF_SIZE);
    }

    return all_write;
}

int copy_directory(char* source, char* destination){
    DIR* dir = opendir(source);
    if(!dir) error("opendir");

    try(mkdir(destination, 0777));
    struct dirent* dp;
    struct stat st;

    while((dp = readdir(dir)) != NULL){
        if(strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;

        char* src_path = join_paths(source, dp->d_name);
        char* dest_path = join_paths(destination, dp->d_name);
       
        try(stat(src_path, &st));
        if(S_ISDIR(st.st_mode)){
            copy_directory(src_path, dest_path);
        } else {
            copy_file(src_path, dest_path);
        }
        // handle links

        free(src_path);
        free(dest_path);
    }
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


char* join_paths(char* path1, char* path2){
    if(path1[strlen(path1)] == '/'){
        path1[strlen(path1)] = 0;
    }

    if(path2[0] == '/'){
        const int p2len = strlen(path2);
        for(int i = 0; i < p2len - 1; i++)
            path2[i] = path2[i + 1];
        path2[p2len] = 0;
    }

    ssize_t buf_size = snprintf(NULL, -1, "%s/%s", path1, path2);
    char* result = (char*)malloc((buf_size+1)*sizeof(char));
    snprintf(result, buf_size, "%s/%s", path1, path2);
    path2[buf_size] = 0;

    return result;
}

char* get_relative_path(char* base_path, char* path){
    const int base_path_len = strlen(base_path);
    const int path_len = strlen(path);
    char* result = (char*)malloc((path_len - base_path_len + 1)*sizeof(char));

    strncpy(result, path + base_path_len, path_len - base_path_len);
    result[path_len - base_path_len] = 0;
    return result;
}

int handle_create_file(char* base_path, char* source, char* destination){
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = copy_file(source, dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_create_dir(char* base_path, char* source, char* destination){
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = mkdir(dest_path, 0777);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_delete_file(char* base_path, char* source, char* destination){
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = remove(dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_delete_dir(char* base_path, char* source, char* destination){
    char* rel_path = get_relative_path(base_path, source);
    char* dest_path = join_paths(destination, rel_path);

    int res = rmdir(dest_path);

    free(rel_path);
    free(dest_path);

    return res;
}

int handle_move(char* base_path, char* source, char* destination, char* new_source){
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