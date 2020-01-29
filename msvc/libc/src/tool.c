#define QUICKJS_USE_IN_TOOL
#include <sys/time.h>
#include <unistd.h>
#include <Windows.h>
#include <ftw.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>

struct _pthread_t
{
    HANDLE handle;

    void* arg;
    void *(*start)(void *);
};

static DWORD _qjs_pthread_start(LPVOID arg)
{
    pthread_t p = (pthread_t)arg;
    p->start(p->arg);
    return 0;
}

int qjs_pthread_create(pthread_t* tid, const pthread_attr_t* attr, void *(*start)(void *), void* arg)
{
    (void) attr;

    *tid = (pthread_t)malloc(sizeof(struct _pthread_t));
    (*tid)->arg = arg;
    (*tid)->start = start;
    (*tid)->handle = CreateThread(NULL, 0, _qjs_pthread_start, *tid, 0, NULL);
    return 0;
}

int qjs_pthread_join(pthread_t thread, void** value_ptr)
{
    WaitForSingleObject(thread->handle, INFINITE);
    CloseHandle(thread->handle);

    free(thread);

    return 0;
}

int usleep(useconds_t usec) 
{ 
    HANDLE timer; 
    LARGE_INTEGER ft; 

    ft.QuadPart = -(10*usec); // Convert to 100 nanosecond interval, negative value indicates relative time
    timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer);

    return 0;
}

#pragma comment(lib, "winmm.lib")

int qjs_clock_gettime2(clockid_t clk_id, struct timespec *tp)
{
    if (clk_id == CLOCK_MONOTONIC)
    {
        DWORD t = timeGetTime();
        tp->tv_sec = t / 1000;
        tp->tv_nsec = (t % 1000) * 1000000;
        return 0;
    }
    else
    {
        errno = ENOTSUP;
        return -1;
    }
}

#define FTW_DNR 4
#define FTW_D 2
#define FTW_NS 3
#define FTW_F 1

#define concat_char(...) _concat_char(__VA_ARGS__, NULL)
static char* _concat_char(const char* a, ...)
{
    va_list va;
    size_t total_length = 1;
    va_start(va, a);
    const char* nextString = a;
    do
    {
        total_length += strlen(nextString);
    } while((nextString = va_arg(va, const char*)) != NULL);
    va_end(va);

    va_start(va, a);
    char* ret = (char*)malloc(total_length * sizeof(char));
    size_t retstart = 0;
    nextString = a;
    do
    {
        size_t len = strlen(nextString);
        memcpy(&ret[retstart], nextString, len * sizeof(char));
        retstart += len;
    } while((nextString = va_arg(va, const char*)) != NULL);
    ret[total_length - 1] = 0;
    va_end(va);

    return ret;
}

int ftw(const char* dirpath,
    int (*fn) (const char* fpath, const struct stat* sb, int typeflag),
    int nopenfd)
{
    struct _finddata_t FindData;
    char* dirpath_ = concat_char(dirpath, "/*");
    intptr_t hList = _findfirst(dirpath_, &FindData);
    free(dirpath_);
    if (hList == -1) return -1;

    do
    {
        if (strcmp(".", FindData.name) == 0 || strcmp("..", FindData.name) == 0) continue;
        else
        {
            char* currpath = concat_char(dirpath, "/", FindData.name);
            
            struct stat file_stat;
#ifdef _USE_32BIT_TIME_T
            int statret = _stat(currpath, (struct _stat32*) & file_stat);
#else
            int statret = _stat(currpath, (struct _stat64i32*) & file_stat);
#endif
            int flag;

            if (statret == 0)
            {
                if (FindData.attrib & _A_SUBDIR)
                {
                    flag = FTW_D;
                }
                else
                {
                    if (FindData.attrib & _A_NORMAL)
                        flag = FTW_F;
                    else
                        flag = FTW_NS;
                }
            }
            else
            {
                flag = FTW_NS;
            }

            int ret = fn(currpath, &file_stat, flag);

            if (flag == FTW_D && ret == 0)
                ret = ftw(currpath, fn, nopenfd - 1);

            free(currpath);

            if (ret != 0) return ret;
        }
    } while (_findnext(hList, &FindData) == 0);

    _findclose(hList);

    return 0;
}