#pragma once

#if defined(QUICKJS_USE_IN_TOOL)

#include <getopt.h>
#include <stdint.h>
#include <process.h>

#define getpid _getpid
#define unlink _unlink

typedef uint32_t useconds_t;

int usleep(useconds_t usec);

#endif

#if defined(QUICKJS_USE_IN_LIBC)

#include_next <unistd.h>
#include <process.h>
#include <direct.h>
#include <io.h>
#include <stdio.h>

FILE* qjs_popen(const char *command, const char *type);
int qjs_open(const char *path, int oflag, ... );
char *qjs_getcwd(char *buf, size_t size);

#define fdopen _fdopen
#define fileno _fileno
#define popen qjs_popen
#define pclose _pclose
#define open qjs_open
#define write _write
#define read _read
#define close _close
#define lseek _lseek
#define isatty _isatty
#define getcwd _getcwd

#define PATH_MAX 260

#endif