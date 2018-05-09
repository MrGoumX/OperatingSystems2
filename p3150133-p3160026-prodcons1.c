//
// Created by mgx on 9/5/2018.
//

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "p3150133-p3160026-prodcons.h"

pthread_mutex_t mutexcount;
pthread_cond_t prod_condition, cons_condition;
int

int main(int argc, char** argv){
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL){
        printf("Not enough arguments given. Exiting...\n");
        exit(-1);
    }
    int number_of_producers;
    int number_of_consumers;
    int size_of_queue;
    int size_of_production;
    int seed;
    circular_buffer *cb;
    number_of_producers = atoi(argv[1]);
    number_of_consumers = atoi(argv[2]);
    size_of_queue = atoi(argv[3]);
    size_of_production = atoi(argv[4]);
    seed = atoi(argv[5]);
    pthread_t producers[number_of_producers];
    pthread_t consumers[number_of_consumers];
    int producers_id[number_of_producers];
    int consumers_id[number_of_consumers];
    int rc, mut, prod_condition_init, cons_condition_init;
    int i;
    cb_init(cb, size_of_queue, sizeof(int));

    mut = pthread_mutex_init(&mutexcount, NULL);
    if(mut != 0){
        printf("Error: %d\n", mut);
        exit(-1);
    }

    prod_condition_init = pthread_cond_init(&prod_condition, NULL);
    if(prod_condition_init != 0){
        printf("Error: %d\n", prod_condition_init);
        exit(-1);
    }

    cons_condition_init = pthread_cond_init(&cons_condition, NULL);
    if(cons_condition_init != 0){
        printf("Error: %d\n", cons_condition_init);
        exit(-1);
    }


    for(i = 0; i < number_of_producers; i++){
        producers_id[i] = i+1;
        printf("Creating producer %i", i+1);
        producer_info *info = malloc(sizeof *info);
        info->id = &producers_id[i];
        info->size_of_production = &size_of_production;
        info->seed = &seed;
        rc = pthread_create(&producers[i], NULL, producer, info);
        if(rc != 0){
            printf("Error: %d\n", rc);
            free(info);
            exit(-1);
        }
    }

    for(i = 0; i < number_of_consumers; i++){
        consumers_id[i] = i+1;
        printf("Creating consumer %i", i+1);
        rc = pthread_create(&consumers[i], NULL, consumer, consumers_id[i]);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
    }

    void *status;
    for(i = 0; i < number_of_producers; i++){
        rc = pthread_join(producers[i], &status);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
    }

    for(i = 0; i < number_of_consumers; i++){
        rc = pthread_join(consumers[i], &status);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
    }

    rc = pthread_mutex_destroy(&mutexcount);
    if(rc != 0){
        printf("Error: %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_destroy(&prod_condition);
    if(rc != 0){
        printf("Error: %d\n", rc);
        exit(-1);
    }

    rc = pthread_cond_destroy(&cons_condition);
    if(rc != 0){
        printf("Error: %d\n", rc);
        exit(-1);
    }

    return 0;
}

void *producer(void *args){

}

void *consumer(void *args){

}