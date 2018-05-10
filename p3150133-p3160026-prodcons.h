//
// Created by mgx on 9/5/2018.
//

#ifndef OPERATINGSYSTEMS2_P3150133_P3160026_PRODCONS_H
#define OPERATINGSYSTEMS2_P3150133_P3160026_PRODCONS_H

#include <stddef.h>

typedef struct circular_buffer
{
    void *buffer;     // data buffer
    void *buffer_end; // end of data buffer
    size_t capacity;  // maximum number of items in the buffer
    size_t count;     // number of items in the buffer
    size_t sz;        // size of each item in the buffer
    void *head;       // pointer to head
    void *tail;       // pointer to tail
} circular_buffer;

typedef struct producer_info
{
    int id;
    int size_of_production;
    unsigned int seed;
} producer_info;

typedef struct thread_ret
{
    int id;
    int count;
    int *consumed;
} thread_ret;


void cb_init(circular_buffer *cb, size_t capacity, size_t sz);

void cb_free(circular_buffer *cb);

void cb_push_back(circular_buffer *cb, const void *item);

void cb_pop_front(circular_buffer *cb, void *item);

void *producer(void *args);

void *consumer(void *args);
#endif //OPERATINGSYSTEMS2_P3150133_P3160026_PRODCONS_H
