#ifndef FILE_H
#define FILE_H

// All paths should be absolute

int copy_file(char* source, char* destination);

int copy_directory(char* source, char* destination);

int ensure_directory_exists(char* path);

char* join_paths(char* path1, char* path2);

char* get_relative_path(char* base_path, char* path);

int handle_create_file(char* base_path, char* source, char* destination);

int handle_create_dir(char* base_path, char* source, char* destination);

int handle_delete_file(char* base_path, char* source, char* destination);

int handle_delete_dir(char* base_path, char* source, char* destination);

int handle_move(char* base_path, char* source, char* destination, char* new_source);

#endif //FILE_H