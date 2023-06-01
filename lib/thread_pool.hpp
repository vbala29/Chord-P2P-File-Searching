/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#pragma once

#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <queue>

#define MAX_THREADS 100

void initMutexAndAttr(pthread_mutex_t& mutex, pthread_mutexattr_t& attr);


template <typename T>
class ThreadPool {
    public:
        /**
         * @brief 
         * 
         */
        typedef void (*workerFunction)(T, void* args);
        
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

        typedef struct {
            ThreadPool* this_tp;
            void* thread_id;
        } ThreadArgs;

        static void* pthread_worker_wrapper(void* object) {
            ThreadArgs* ta = reinterpret_cast<ThreadArgs*>(object);
            ta->this_tp->worker_wrapper(ta->thread_id);
            free(ta);
            return NULL;
        }
        
        /**
         * @brief Destroy the Thread Pool object
         * 
         */
        ~ThreadPool();

        void assign_job();

    private:
        pthread_mutex_t job_queue_lock;
        std::queue<T> jobs; //Jobs to execute
        std::vector<pthread_t*> threads; //All available threads in the pool
        pthread_t* assigner_thread; //Assigns jobs to threads in the pool

        std::vector<T> jobAssignments; //Jobs assigned to threads. A job is a new assignment if corresponding the thread_dispatch_indicator is set to true.
        std::vector<pthread_mutex_t*> thread_mutexes; //To control accessing of thread_dispatch_indicators
        std::vector<pthread_cond_t*> thread_cond_vars;
        std::vector<bool> thread_dispatch_indicators; //false indicates that thread is done and waiting for new job. true indicates that thread should take a job from queue/thread is running with a job.
        workerFunction wf; // The function that the user of this library wishes to execute in each Thread. 

        /**
         * @brief 
         * 
         * @param args 
         * @return void* 
         */
        void* worker_wrapper(void* args);

        

};