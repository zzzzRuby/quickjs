#pragma once

#include_next <pthread.h>

typedef struct _pthread_t* pthread_t;
typedef void* pthread_attr_t;

int qjs_pthread_create(pthread_t* tid, const pthread_attr_t* attr, void *(*start)(void *), void* arg);
int qjs_pthread_join(pthread_t thread, void** value_ptr);

#define pthread_create qjs_pthread_create
#define pthread_join qjs_pthread_join