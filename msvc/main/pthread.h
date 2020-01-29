#pragma once

#include <sys/time.h>
#include <Windows.h>

typedef struct _pthread_mutex_t { CRITICAL_SECTION cs;UINT level; } pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;
typedef void* pthread_condattr_t;

#define PTHREAD_MUTEX_LEVEL_UNINIT 0
#define PTHREAD_MUTEX_LEVEL_INIT 1

#define PTHREAD_COND_INITIALIZER RTL_CONDITION_VARIABLE_INIT
#define PTHREAD_MUTEX_INITIALIZER { { (void*)-1, -1, 0, 0, 0, 0 }, PTHREAD_MUTEX_LEVEL_UNINIT }

int qjs_pthread_mutex_lock(pthread_mutex_t *mutex);
int qjs_pthread_mutex_unlock(pthread_mutex_t *mutex);
int qjs_pthread_cond_signal(pthread_cond_t *cond);
int qjs_pthread_cond_destroy(pthread_cond_t *cond);
int qjs_pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex);
int qjs_pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr);
int qjs_pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);
int qjs_pthread_cond_broadcast(pthread_cond_t* cond);

#define pthread_mutex_lock qjs_pthread_mutex_lock
#define pthread_mutex_unlock qjs_pthread_mutex_unlock
#define pthread_cond_signal qjs_pthread_cond_signal
#define pthread_cond_destroy qjs_pthread_cond_destroy
#define pthread_cond_wait qjs_pthread_cond_wait
#define pthread_cond_init qjs_pthread_cond_init
#define pthread_cond_timedwait qjs_pthread_cond_timedwait
#define pthread_cond_broadcast qjs_pthread_cond_broadcast