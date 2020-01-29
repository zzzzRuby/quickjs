#include <pthread.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <stdint.h>

static __forceinline uint64_t FileTimeToUint64(FILETIME* ft)
{
    ULARGE_INTEGER result;

    result.LowPart = ft->dwLowDateTime;
    result.HighPart = ft->dwHighDateTime;
    return result.QuadPart;
}

static __forceinline void qjs_timespec_offset(struct timespec* tp, LARGE_INTEGER* freq, LARGE_INTEGER* counter)
{
    tp->tv_sec += counter->QuadPart / freq->QuadPart;
    LONGLONG nsec = tp->tv_nsec + ((counter->QuadPart % freq->QuadPart) * 1000000000 + (freq->QuadPart >> 1)) / freq->QuadPart;
    while (nsec >= 1000000000) {
        tp->tv_sec ++;
        nsec -= 1000000000;
    }
    tp->tv_nsec = (long)nsec;
}

static __forceinline void qjs_gettimeofday_internal(struct timespec *tp)
{
	static const uint64_t offset = 116444736000000000ull;
    static BOOL isinitialized = FALSE;
    static uint64_t base_time;
    static LARGE_INTEGER freq, base_counter;

    if (!isinitialized)
    {
        FILETIME ft;
	    GetSystemTimePreciseAsFileTime(&ft);
        QueryPerformanceCounter(&base_counter);
        QueryPerformanceFrequency(&freq);
        base_time = FileTimeToUint64(&ft) - offset;
        isinitialized = TRUE;
    }

    if (tp)
    {
        LARGE_INTEGER now_counter, offset_counter;
        QueryPerformanceCounter(&now_counter);
        offset_counter.QuadPart = now_counter.QuadPart - base_counter.QuadPart;
        
	    tp->tv_sec = (time_t)(base_time / 10000000);
	    tp->tv_nsec = (long)((base_time % 10000000) * 100);
        qjs_timespec_offset(tp, &freq, &offset_counter);
    }
}

int qjs_gettimeofday(struct timeval* tp, void* tzp)
{
    struct timespec t;
    qjs_gettimeofday_internal(&t);
    tp->tv_sec = (long)t.tv_sec;
    tp->tv_usec = t.tv_nsec / 1000;
    return 0;
}

int qjs_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    if (clk_id == CLOCK_REALTIME)
    {
        tp->tv_nsec = 0;
        tp->tv_sec = 0;
        return 0;
    }
    else
    {
	    errno = ENOTSUP;
	    return -1;
    }
}

static __forceinline void _qjs_pthread_check_mutex(pthread_mutex_t* mutex)
{
    if (mutex->level == PTHREAD_MUTEX_LEVEL_UNINIT)
    {
        InitializeCriticalSection(&mutex->cs);
        mutex->level = PTHREAD_MUTEX_LEVEL_INIT;
    }
}

int qjs_pthread_mutex_lock(pthread_mutex_t *mutex)
{
    _qjs_pthread_check_mutex(mutex);
	EnterCriticalSection(&mutex->cs);
    return 0;
}

int qjs_pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    _qjs_pthread_check_mutex(mutex);
	LeaveCriticalSection(&mutex->cs);
	return 0;
}

int qjs_pthread_cond_signal(pthread_cond_t *cond)
{
	WakeConditionVariable(cond);
	return 0;
}

int qjs_pthread_cond_destroy(pthread_cond_t *cond)
{
	(void) cond;
	return 0;
}

int qjs_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    _qjs_pthread_check_mutex(mutex);
	SleepConditionVariableCS(cond, &mutex->cs, INFINITE);
	return 0;
}

int qjs_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr)
{
	(void) attr;
	InitializeConditionVariable(cond);
	return 0;
}

static __forceinline DWORD timespec_to_ms(const struct timespec *abstime)
{
    if (abstime == NULL)
        return INFINITE;

    return (abstime->tv_sec * 1000) + (abstime->tv_nsec / 1000000);
}

int qjs_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime)
{
    _qjs_pthread_check_mutex(mutex);
	
	if (!SleepConditionVariableCS(cond, &mutex->cs, timespec_to_ms(abstime))) return ETIMEDOUT;
	
	return 0;
}

int qjs_pthread_cond_broadcast(pthread_cond_t* cond)
{
	WakeAllConditionVariable(cond);
	return 0;
}