#pragma once

#include <Windows.h>
#include <time.h>

typedef int clockid_t;

#define CLOCK_REALTIME    1

int qjs_gettimeofday(struct timeval *tv, void* tz);
int qjs_clock_gettime(clockid_t clk_id, struct timespec *tp);

#define gettimeofday qjs_gettimeofday
#define clock_gettime qjs_clock_gettime