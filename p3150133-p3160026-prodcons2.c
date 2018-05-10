#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include "p3150133-p3160026-prodcons.h"

circular_buffer circ_buff; // Circular Buffer
pthread_mutex_t mutex; // Mutex for the buffer
pthread_cond_t prod_condition, cons_condition; // Conditions for producers and consumers
int counter, cons; // counter: general counter for the consumers oreration, cons: total amount of item to be produced
FILE *in, *out; // in: prod_in.txt, out: cons_out.txt

//main thread
int main(int argc, char** argv){
    //Check for valid arguments
    if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL || argv[6] != NULL){
        printf("Invalid arguments given. Exiting...\n");
        exit(-1);
    }
    //Variable definition
    int number_of_producers;
    int number_of_consumers;
    int size_of_queue;
    int size_of_production;
    unsigned int seed;
    //Variable initialization
    number_of_producers = atoi(argv[1]);
    number_of_consumers = atoi(argv[2]);
    size_of_queue = atoi(argv[3]);
    size_of_production = atoi(argv[4]);
    seed = strtoul(argv[5], 0L, 10);
    //procucer array threads
    pthread_t producers[number_of_producers];
    //consumer array threads
    pthread_t consumers[number_of_consumers];
    //producers ids
    int producers_id[number_of_producers];
    //consumers ids
    int consumers_id[number_of_consumers];
    //error codes
    int rc, mut, prod_condition_init, cons_condition_init;
    int i;
    //counter initialization for consumers operation
    counter = 0;
    //cons: total number of items to be produced
    cons = number_of_producers*size_of_production;

    //files creation
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

    //circular buffer initialization
    cb_init(&circ_buff, size_of_queue, sizeof(int));

    //mutex initialization
    mut = pthread_mutex_init(&mutex, NULL);
    if(mut != 0){
        printf("Error: %d\n", mut);
        exit(-1);
    }

    //producers condition initialization
    prod_condition_init = pthread_cond_init(&prod_condition, NULL);
    if(prod_condition_init != 0){
        printf("Error: %d\n", prod_condition_init);
        exit(-1);
    }

    //consumers condition initialization
    cons_condition_init = pthread_cond_init(&cons_condition, NULL);
    if(cons_condition_init != 0){
        printf("Error: %d\n", cons_condition_init);
        exit(-1);
    }

    //producers threads initialization
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

    //consumers threads initialization
    for(i = 0; i < number_of_consumers; i++){
        consumers_id[i] = i+1;
        printf("Creating consumer %i\n", i+1);
        rc = pthread_create(&consumers[i], NULL, consumer, consumers_id[i]);
        if(rc != 0){
            printf("Error: %d\n", rc);
            exit(-1);
        }
    }

    //producers threads join
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

    //consumers threads join
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

    //free memory for later prodconsx.c
    for(i = 0; i < number_of_producers; i++){
        free(prod_ret[i].consumed);
    }

    //free memory for later prodconsx.c
    for(i = 0; i < number_of_consumers; i++){
        free(cons_ret[i].consumed);
    }

    //close files
    fclose(in);
    fclose(out);

    //destroy mutex
    rc = pthread_mutex_destroy(&mutex);
    if(rc != 0){
        printf("Error: %d\n", rc);
        exit(-1);
    }

    //destroy producers condtion
    rc = pthread_cond_destroy(&prod_condition);
    if(rc != 0){
        printf("Error: %d\n", rc);
        exit(-1);
    }

    //destroy consumers condition
    rc = pthread_cond_destroy(&cons_condition);
    if(rc != 0){
        printf("Error: %d\n", rc);
        exit(-1);
    }

    //free circular queue
    cb_free(&circ_buff);

    //exit
    return 0;
}

//producer thread
void *producer(void *args){
    //receive arguments
    producer_info *r_args = (producer_info *) args;
    //variable initialization
    int id = r_args->id;
    int size_of_production = r_args->size_of_production;
    unsigned int seed = r_args->seed*id;
    //array for holding the data produced
    int *produced = malloc(sizeof(int)*size_of_production);
    int rc;
    //production
    for(int i = 0; i < size_of_production; i++){
        //lock the buffer
        pthread_mutex_lock(&mutex);
        //if buffer is full trigger the mutex for the consumers
        while(circ_buff.capacity == circ_buff.count){
            rc = pthread_cond_wait(&prod_condition, &mutex);
            if(rc != 0){
                printf("Error: %d\n", rc);
                pthread_exit(&rc);
            }
            printf("The queue is full\n");
        }
        //produce random
        int ran = rand_r(&seed);
        //print to file
        fprintf(in, "Producer %d: %d\n", id, ran);
        //save to array
        produced[i] = ran;
        //save to buffer
        cb_push_back(&circ_buff, &ran);
        //broadcast to other production threads
        rc = pthread_cond_broadcast(&prod_condition);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        //broadcast to consumer threads
        pthread_cond_broadcast(&cons_condition);
        //unlock the buffer
        rc = pthread_mutex_unlock(&mutex);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        //sleep for 1 ms
        sleep(1);
    }
    //print to console
    printf("Producer %d: ", id);
    for(int j = 0; j < size_of_production; j++) {
        printf("%d", produced[j]);
        printf((j != size_of_production - 1) ? ", " : "\n");
    }
    //free memory from arguments
    free(r_args);
    //for later prodconsx.c use
    thread_ret *info = malloc(sizeof(*info));
    info->id = id;
    info->count = size_of_production;
    info->consumed = produced;
    //exit thread
    pthread_exit(info);

}

//consumer thread
void *consumer(void *args){
    //receive arguments
    int *id = (int *) args;
    //variable initialization
    int rc;
    //thr_count: the total amount of numbers consumed from that thread
    int thr_count = 0;
    //array for data consumption
    int *consumed = malloc(sizeof(int)+1);
    //if the general counter is less than the total production iterate
    while(counter++ < cons){
        //lock the buffer
        pthread_mutex_lock(&mutex);
        //if the buffer is empty trigger the production threads
        while(circ_buff.count == 0){
            rc = pthread_cond_wait(&cons_condition, &mutex);
            if(rc != 0){
                printf("Error: %d\n", rc);
                pthread_exit(&rc);
            }
            printf("The queue is empty\n");
        }
        //consume number
        int poped;
        //pop from queue
        cb_pop_front(&circ_buff, &poped);
        //print to file
        fprintf(out, "Consumer %d: %d\n", id, poped);
        //save to array
        consumed[thr_count] = poped;
        //realloc space for 1 more number
        thr_count++;
        consumed = realloc(consumed, thr_count*sizeof(int)+1);
        //broadcast to other consumer threads
        rc = pthread_cond_broadcast(&cons_condition);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        //broadcast to production threads
        pthread_cond_broadcast(&prod_condition);
        //unlock the buffer
        rc = pthread_mutex_unlock(&mutex);
        if(rc != 0){
            printf("Error: %d\n", rc);
            pthread_exit(&rc);
        }
        //sleep for 1 ms
        sleep(1);
    }
    //print to console
    printf("Consumer %d: ", id);
    for(int j = 0; j < thr_count; j++){
        printf("%d", consumed[j]);
        printf((j != thr_count-1) ? ", " : "\n");
    }
    //for later prodconsx.c use
    thread_ret *info = malloc(sizeof(*info));
    info->id = id;
    info->count = thr_count;
    info->consumed = consumed;
    //exit thread
    pthread_exit(info);

}