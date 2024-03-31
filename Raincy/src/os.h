#pragma once
#include <stdbool.h>

char* os_get_home();
bool os_folder_exists(const char* path);
bool os_folder_create(const char* path);
bool os_path_exists(const char* path);



