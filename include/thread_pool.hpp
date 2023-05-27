/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */


#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 100

template <typename T>
class ThreadPool {
    public:
        ThreadPool(uint16_t num_workers) {
            if (pthread_mutexattr_init(&mutex_attributes) != 0) {
                perror("pthread_mutex_attr_init() error");
                exit(1);
            } 
            
            if (pthread_mutexattr_settype(&mutex_attributes, PTHREAD_MUTEX_ERRORCHECK) != 0) {
                perror("pthread_mutexattrr_settype() error");
                exit(1);
            }
            
            if (pthread_mutex_init(&lock, &mutex_attributes) != 0) {
                perror("pthread_mutex_init() error");
                exit(1);
            }

            if (pthread_cond_init(&cond_var, NULL) != 0) {
                perror("pthread_cond_init() error");
                exit(1);
            }

            for (int i = 0; i < min(MAX_THREADS, num_workers); i++) {
                pthread_t* thread_ID = (pthread_t*) malloc(sizeof(pthread_t);
                if (thread == NULL) {
                    perror("malloc error for pthread_t");
                    exit(1);
                }

                pthread_create(&thread_ID, NULL, worker_function, NULL);
            }
        }
    

    private:
        std::queue<T> jobs;
        std::vector<pthread_t> threads;
        pthread_mutex_t lock;
        pthread_mutexattr_t mutex_attributes;
        pthread_cond_t cond_var;


};