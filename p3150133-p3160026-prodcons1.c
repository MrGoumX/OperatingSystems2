//
// Created by mgx on 9/5/2018.
//

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "p3150133-p3160026-prodcons.h"

circular_buffer circ_buff;
pthread_mutex_t mutex;
pthread_cond_t prod_condition, cons_condition;
int finished = 0;
int prod;
int prods, cons;

int main(int argc, char** argv){
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL){
        printf("Not enough arguments given. Exiting...\n");
        exit(-1);
    }
    int number_of_producers;
    int number_of_consumers;
    int size_of_queue;
    int size_of_production;
    unsigned int seed;
    number_of_producers = atoi(argv[1]);
    number_of_consumers = atoi(argv[2]);
    size_of_queue = atoi(argv[3]);
    size_of_production = atoi(argv[4]);
    seed = strtoul(argv[5], 0L, 10);
    pthread_t producers[number_of_producers];
    pthread_t consumers[number_of_consumers];
    int producers_id[number_of_producers];
    int consumers_id[number_of_consumers];
    int rc, mut, prod_condition_init, cons_condition_init;
    int i;
    prod = number_of_producers;
    printf("%d", size_of_queue);
    cb_init(&circ_buff, size_of_queue, sizeof(int));

    mut = pthread_mutex_init(&mutex, NULL);
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
        printf("Creating producer %i\n", i+1);
        /*producer_info *info = malloc(sizeof *info);
        info->id = &producers_id[i];
        info->size_of_production = &size_of_production;
        info->seed = &seed;*/
        producer_info info;
        printf("%d\n", producers_id[i]);
        info.id = producers_id[i];
        info.size_of_production = size_of_production;
        info.seed = seed;
        rc = pthread_create(&producers[i], NULL, producer, &info);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
    }

    for(i = 0; i < number_of_consumers; i++){
        consumers_id[i] = i+1;
        printf("Creating consumer %i\n", i+1);
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

    rc = pthread_mutex_destroy(&mutex);
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
    cb_free(&circ_buff);
    return 0;
}

void *producer(void *args){
    producer_info *r_args = (producer_info *) args;
    int id = r_args->id;
    int size_of_production = r_args->size_of_production;
    unsigned int seed = r_args->seed;
    int produced[size_of_production];
    int rc;
    for(int i = 0; i < size_of_production; i++){
        pthread_mutex_lock(&mutex);
        while(prods == cons){
            rc = pthread_cond_wait(&prod_condition, &mutex);
            if(rc != 0){
                printf("Error: %d\n", rc);
                pthread_exit(&rc);
            }
            printf("The queue is full\n");
        }
        int ran = rand_r(&seed);
        printf("Producer %d: %d\n", id, ran);
        produced[i] = ran;
        cb_push_back(&circ_buff, &ran);
        rc = pthread_cond_broadcast(&prod_condition);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        pthread_cond_signal(&cons_condition);
        rc = pthread_mutex_unlock(&mutex);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        sleep(1);
    }
    finished++;
    //return (void *)produced;
    pthread_exit(id);

}

void *consumer(void *args){
    int *id = (int *) args;
    int rc;
    int *consumed = malloc(sizeof(int));
    int counter = 0;
    while(circ_buff.head != circ_buff.buffer || circ_buff.head == circ_buff.buffer){
        pthread_mutex_lock(&mutex);
        while(circ_buff.count == 0){
            rc = pthread_cond_wait(&cons_condition, &mutex);
            if(rc != 0){
                printf("Error: %d\n", rc);
                pthread_exit(&rc);
            }
            printf("The queue is empty\n");
        }
        int poped;
        cb_pop_front(&circ_buff, &poped);
        printf("Consumer %d: %d\n", id, poped);
        consumed[counter] = poped;
        counter++;
        realloc(consumed, sizeof(consumed)* sizeof(int) + 1*sizeof(int));
        rc = pthread_cond_broadcast(&cons_condition);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        pthread_cond_signal(&prod_condition);
        rc = pthread_mutex_unlock(&mutex);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        sleep(1);
    }
    int cons_stat[counter];
    for(int i = 0; i < counter; i++){
        cons_stat[i] = consumed[i];
    }
    free(consumed);
    consumer_info *info = malloc(sizeof(*info));
    info->count = counter;
    info->consumed = cons_stat;
    //return (void *) info;
    pthread_exit(id);

}