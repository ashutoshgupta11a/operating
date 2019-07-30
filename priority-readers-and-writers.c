#include "priority-readers-and-writers.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

unsigned int shared_variable = 0;
//pthread_mutex_t lock;
//int pthread_mutex_init(pthread_mutex_t *restrict mutex,const pthread_mutexattr_t *restrict attr);
//pthread_mutex_init(&lock, NULL);
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t read_phase = PTHREAD_COND_INITIALIZER;
pthread_cond_t write_phase = PTHREAD_COND_INITIALIZER;
int waiting_readers = 0, readers = 0;

int main(int argc, char **argv) {
    int i = 0;
    int reader_num[5];
    int writer_num[5];

    pthread_t reader_thread_ids[5];
    pthread_t writer_thread_ids[5];
  
    // Seed the random number generator
    // void srandom(unsigned int seed);
    srandom((unsigned int)time(NULL));

    for(i=0; i < 5; i++){
        reader_num[i] = i;
        //pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
        pthread_create(&reader_thread_ids[i], NULL, readerMain, &reader_num[i]);
    }

    for(i=0; i<5; i++){
        writer_num[i] = i;
        //pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start)(void *), void *arg);
        pthread_create(&writer_thread_ids[i], NULL, writerMain, &writer_num[i]);
     }

    for(i=0; i < 5; i++){
        //int pthread_join(pthread_t thread, void **value_ptr);
        pthread_join(reader_thread_ids[i], NULL);
    }

    for(i=0; i < 5; i++){
        //int pthread_join(pthread_t thread, void **value_ptr);
        pthread_join(writer_thread_ids[i], NULL);
    }

    return 0;
}

//code for reader
void *readerMain(void *thread_args){

    int num_readers = 0;
    int id = *((int*)thread_args);
    int i = 0;
    for(i = 0; i < 5; i++) {
        usleep(1000 * (random() % 5 + 5));

      //enter critical section
        pthread_mutex_lock(&lock);
            waiting_readers++;
            while(readers == -1) {
                //pthread_cond_wait(pthread_cond_t *restrict cond,pthread_mutex_t *restrict mutex);
                pthread_cond_wait(&read_phase, &lock);
            }
            waiting_readers--;
            num_readers = ++readers;
        pthread_mutex_unlock(&lock);

        //read
        fprintf(stdout, "[r%d] reading %u  [readers: %2d]\n", id, shared_variable, readers);

        //exit critical critical section
        pthread_mutex_lock(&lock);
            readers--;
            if (readers == 0) {
                pthread_cond_signal(&write_phase);
            }
        pthread_mutex_unlock(&lock);
        }
    pthread_exit(0);
}

void *writerMain(void *thread_args){

    int num_readers = 0;
    int id = *((int*)thread_args);
    int i = 0;
    
    for(i = 0; i < 5; i++) {
        usleep(1000 * (random() % 5 + 5));
  
        //enter critical section
        pthread_mutex_lock(&lock);
        while(readers !=  0) {
            //pthread_cond_wait(pthread_cond_t *restrict cond,pthread_mutex_t *restrict mutex);
            pthread_cond_wait(&write_phase, &lock);
        }
        readers = -1;
        num_readers = readers;
        pthread_mutex_unlock(&lock);

        //read
        fprintf(stdout, "[w%d] writer %u  [writers: %2d]\n", id, ++shared_variable, readers);

        //exit critical critical section
        pthread_mutex_lock(&lock);
            readers = 0;
            if (waiting_readers > 0) {
                pthread_cond_broadcast(&read_phase);
            }
            else {
                pthread_cond_signal(&write_phase);
            }
        pthread_mutex_unlock(&lock);
        }
    pthread_exit(0);
}
