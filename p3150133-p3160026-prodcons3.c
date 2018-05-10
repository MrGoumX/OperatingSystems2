//TODO: ADD COMMENTS
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "p3150133-p3160026-prodcons.h"

circular_buffer circ_buff;
pthread_mutex_t mutex;
pthread_cond_t prod_condition, cons_condition;
int counter, cons;
FILE *in, *out;

int main(int argc, char** argv){
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL || argv[6] != NULL){
        printf("Invalid arguments given. Exiting...\n");
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
    counter = 0;
    cons = number_of_producers*size_of_production;

    in = fopen("prod_in.txt", "w");
    if(in == NULL){
        printf("ERROR, opening file!\n");
        exit(-1);
    }

    out = fopen("cons_out.txt", "w");
    if(out == NULL){
        printf("ERROR, opening file!\n");
        exit(-1);
    }

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
        producer_info *info = malloc(sizeof(*info));
        info->id = producers_id[i];
        info->size_of_production = size_of_production;
        info->seed = seed;
        rc = pthread_create(&producers[i], NULL, producer, info);
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

    thread_ret prod_ret[number_of_producers];
    thread_ret cons_ret[number_of_consumers];
    void *status;
    for(i = 0; i < number_of_producers; i++){
        thread_ret ret;
        rc = pthread_join(producers[i], &status);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
        ret = *(thread_ret*)status;
        prod_ret[i] = ret;
    }

    for(i = 0; i < number_of_consumers; i++){
        thread_ret ret;
        rc = pthread_join(consumers[i], &status);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
        ret = *(thread_ret*)status;
        cons_ret[i] = ret;
    }
    int k, l, min_idx;
    for (k = 0; k < number_of_producers-1; k++) {
        min_idx = k;
        for (l = k+1; l < number_of_producers; l++) {
            if (prod_ret[l].id < prod_ret[min_idx].id) {
                min_idx = l;
            }
        }
        thread_ret *temp = &prod_ret[i];
        prod_ret[i] = prod_ret[min_idx];
        prod_ret[min_idx] = *temp;
    }

    for (k = 0; k < number_of_consumers-1; k++) {
        min_idx = k;
        for (l = k+1; l < number_of_consumers; l++) {
            if (cons_ret[l].id < cons_ret[min_idx].id) {
                min_idx = l;
            }
        }
        thread_ret *temp = &cons_ret[i];
        cons_ret[i] = cons_ret[min_idx];
        cons_ret[min_idx] = *temp;
    }

    for(i = 0; i < number_of_producers; i++){
        printf("Producer %d: ", prod_ret[i].id);
        for(int j = 0; j < size_of_production; j++){
            printf("%d", prod_ret[i].consumed[j]);
            printf((j != size_of_production-1) ? ", " : "\n");
        }
    }

    for(i = 0; i < number_of_consumers; i++){
        printf("Consumer %d: ", cons_ret[i].id);
        int fin = cons_ret[i].count;
        for(int j = 0; j < fin; j++){
            printf("%d", cons_ret[i].consumed[j]);
            printf((j != fin-1) ? ", " : "\n");
        }
    }

    for(i = 0; i < number_of_producers; i++){
        free(prod_ret[i].consumed);
    }

    for(i = 0; i < number_of_consumers; i++){
        free(cons_ret[i].consumed);
    }

    fclose(in);
    fclose(out);

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
    unsigned int seed = r_args->seed*id;
    int *produced = malloc(sizeof(int)*size_of_production);
    int rc;
    for(int i = 0; i < size_of_production; i++){
        pthread_mutex_lock(&mutex);
        while(circ_buff.capacity == circ_buff.count){
            rc = pthread_cond_wait(&prod_condition, &mutex);
            if(rc != 0){
                printf("Error: %d\n", rc);
                pthread_exit(&rc);
            }
            printf("The queue is full\n");
        }
        int ran = rand_r(&seed);
        fprintf(in, "Producer %d: %d\n", id, ran);
        produced[i] = ran;
        cb_push_back(&circ_buff, &ran);
        rc = pthread_cond_broadcast(&prod_condition);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        pthread_cond_broadcast(&cons_condition);
        rc = pthread_mutex_unlock(&mutex);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        sleep(1);
    }
    free(r_args);
    thread_ret *info = malloc(sizeof(*info));
    info->id = id;
    info->count = size_of_production;
    info->consumed = produced;
    pthread_exit(info);

}

void *consumer(void *args){
    int *id = (int *) args;
    int rc;
    int thr_count = 0;
    int *consumed = malloc(sizeof(int)+1);
    while(counter++ < cons){
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
        fprintf(out, "Consumer %d: %d\n", id, poped);
        consumed[thr_count] = poped;
        thr_count++;
        consumed = realloc(consumed, thr_count*sizeof(int)+1);
        rc = pthread_cond_broadcast(&cons_condition);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        pthread_cond_broadcast(&prod_condition);
        rc = pthread_mutex_unlock(&mutex);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        sleep(1);
    }
    thread_ret *info = malloc(sizeof(*info));
    info->id = id;
    info->count = thr_count;
    info->consumed = consumed;
    pthread_exit(info);

}