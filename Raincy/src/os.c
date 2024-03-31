#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef TARGET_PLATFORM_WINDOWS
#include <windows.h>
#endif

char fmtOsHomeBuffer[1024];
char* os_get_home() {
#ifdef TARGET_PLATFORM_WINDOWS
	//snprintf(fmtOsHomeBuffer, sizeof(fmtOsHomeBuffer), "C:\\Users\\%s", getenv("USERPROFILE"));
	return getenv("USERPROFILE");
#elif defined(TARGET_PLATFORM_LINUX)
	return getenv("HOME");
#else
#error "TODO: get_home() support for other platforms"
#endif
}

bool os_folder_exists(const char* path) {
	DWORD atr = GetFileAttributesA(path);
	return (
		atr != INVALID_FILE_ATTRIBUTES && 
		(atr & FILE_ATTRIBUTE_DIRECTORY)
		);
}
bool os_folder_create(const char* path) {
	return CreateDirectoryA(path, 0);
}
bool os_path_exists(const char* path) {
	DWORD atr = GetFileAttributesA(path);
	return atr != INVALID_FILE_ATTRIBUTES;
}