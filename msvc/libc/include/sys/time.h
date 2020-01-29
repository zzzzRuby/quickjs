#pragma once

#include_next <sys/time.h>

#define CLOCK_MONOTONIC   2

int qjs_clock_gettime2(clockid_t clk_id, struct timespec *tp);

#undef clock_gettime
#define clock_gettime qjs_clock_gettime2