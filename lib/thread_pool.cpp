/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "../include/thread_pool.hpp"


template <typename T>
ThreadPool<T>::ThreadPool(uint16_t num_workers, workerFunction wf) {
    if (wf == NULL) {
        perror("workerFunction wf was null");
        exit(1);
    }
    this->wf = wf;

    if (pthread_mutexattr_init(&mutex_attributes) != 0) {
        perror("pthread_mutex_attr_init() error");
        exit(1);
    } 
    
    if (pthread_mutexattr_settype(&mutex_attributes, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        perror("pthread_mutexattr_settype() error");
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
        pthread_t* thread_ID = (pthread_t*) malloc(sizeof(pthread_t));
        if (thread == NULL) {
            perror("malloc error for pthread_t");
            exit(1);
        }

        pthread_create(&thread_ID, NULL, worker_wrapper, NULL);
        threads.push_back(thread_ID);
    }
}

template <typename T>
void ThreadPool<T>::add_job(T job) {
    pthread_mutex_lock(&lock);
    jobs.push_back(job);
    pthread_mutex_unlock(&lock);
}

template <typename T>
ThreadPool<T>::~ThreadPool() {
    for (pthread_t* t : threads) {
        //free dynamically allocated stuff
    }
}


template <typename T>
void* ThreadPool<T>::worker_wrapper(void* args) {
    for (;;) {
        pthread_mutex_lock(&lock); //Prevent access to job queue
        if (jobs.empty()) pthread_cond_wait(&cond_var, &lock);
        


    }
    



}

