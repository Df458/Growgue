#include <string.h>
#ifdef __MINGW32__
#include <windows.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#elif __GNUC__
#include <unistd.h>
#include <sys/stat.h>
#endif
#include "loader.h"

char* create_path(const char* path)
{
    char* buf = calloc(2048, sizeof(char));
#ifdef __MINGW32__
    GetModuleFileName(NULL, buf, 2048);
    char* c = strrchr(buf, '/');
    c[1] = 0;
#elif __GNUC__
    ssize_t len = readlink("/proc/self/exe", buf, 2047);
    if (len != -1) {
        char* c = strrchr(buf, '/');
        c[1] = 0;
    }
#endif

    char* new_path = calloc(strlen(buf) + strlen(path), sizeof(char));
    strcat(new_path, buf);
    strcat(new_path, path);
    free(buf);
    return new_path;
}

xmlDocPtr load_xml(const char* path)
{
    char* full_path = create_path(path);
    xmlDocPtr doc = xmlReadFile(path, NULL, 0);
    free(full_path);
    return doc;
}
