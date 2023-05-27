/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "thread_pool.hpp"
#include "../src/chord_job.hpp"


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

    uint16_t thread_count = std::min((uint16_t) MAX_THREADS, num_workers);
    for (int i = 0; i < thread_count; i++) {
        pthread_t* thread_ID = (pthread_t*) malloc(sizeof(pthread_t));
        if (thread_ID == NULL) {
            perror("malloc error for pthread_t");
            exit(1);
        }
        void* num =  (void*)(intptr_t) i; 
        ThreadArgs* ta = (ThreadArgs*) malloc(sizeof(ThreadArgs));
        if (ta == NULL) {
            perror("Malloc issue for ThreadArgs");
            exit(1);
        }
        ta->this_tp = this;
        ta->thread_id = num;

        pthread_create(thread_ID, NULL, &ThreadPool::pthread_worker_wrapper, ta);
        threads.push_back(thread_ID);
    }
}

template <typename T>
void ThreadPool<T>::add_job(T job) {
    pthread_mutex_lock(&lock);
    jobs.push(job);
    pthread_mutex_unlock(&lock);
    
    pthread_cond_signal(&cond_var);
}

template <typename T>
ThreadPool<T>::~ThreadPool() {
    for (pthread_t* t : threads) {
        if (pthread_join(*t, NULL) != 0) {
            perror("pthread_join() faced an error, continuing to join other threads");
        }
        //free dynamically allocated stuff
        free(t);
    }

    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond_var);
    pthread_mutexattr_destroy(&mutex_attributes);
}


template <typename T>
void* ThreadPool<T>::worker_wrapper(void* args) {
    for (;;) {
        pthread_mutex_lock(&lock); //Prevent access to job queue
        if (jobs.empty()) pthread_cond_wait(&cond_var, &lock);
        T job_to_execute = jobs.front();
        jobs.pop();
        pthread_mutex_unlock(&lock);

        //Execute the job
        this->wf(job_to_execute, args);
    }
    return NULL;
}

template class ThreadPool<ChordJob>;