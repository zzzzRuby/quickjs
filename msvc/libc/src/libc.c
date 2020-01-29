#define QUICKJS_USE_IN_LIBC
#include <unistd.h>
#include <stdarg.h>
#include <Windows.h>
#include <stdlib.h>
#include <wchar.h>

wchar_t* qjs_utf8_to_wchar(const char* utf8)
{
    wchar_t* result;
    int numChar = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    result = (wchar_t*)malloc(numChar * sizeof(wchar_t));
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, result, numChar);
    return result;
}

FILE* qjs_popen(const char *command, const char *type)
{
    wchar_t* command_w = qjs_utf8_to_wchar(command);
    wchar_t* type_w = qjs_utf8_to_wchar(type);

    FILE* result = _wpopen(command_w, type_w);

    free(command_w);
    free(type_w);

    return result;
}

int qjs_open(const char *path, int oflag, ... )
{
    va_list va;
    wchar_t* path_w = qjs_utf8_to_wchar(path);

    va_start(va, oflag);
    int result = _wopen(path_w, oflag, va);
    va_end(va);

    free(path_w);

    return result;
}

char *qjs_getcwd(char *buf, size_t size);