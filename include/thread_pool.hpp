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
        /**
         * @brief 
         * 
         */
        typedef void (*workerFunction)(T);
        
        /**
         * @brief Construct a new Thread Pool object
         * 
         * @param num_workers 
         * @param wf 
         */
        ThreadPool(uint16_t num_workers, workerFunction wf);

        /**
         * @brief 
         * 
         * @param job 
         */
        void add_job(T job);
        
        /**
         * @brief Destroy the Thread Pool object
         * 
         */
        ~ThreadPool();


    private:
        std::queue<T> jobs; //Jobs to execute
        std::vector<pthread_t*> threads; //All available threads in the pool

        pthread_mutex_t lock; 
        pthread_mutexattr_t mutex_attributes; // For job queue sync
        pthread_cond_t cond_var; // For job queue sync
        workerFunction wf; // The function that the user of this library wishes to execute in each Thread. 

        /**
         * @brief 
         * 
         * @param args 
         * @return void* 
         */
        void* worker_wrapper(void* args);


};