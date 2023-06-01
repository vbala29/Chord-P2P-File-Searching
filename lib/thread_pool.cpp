/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "thread_pool.hpp"
#include "../src/chord_job.hpp"
#include <unistd.h>

void initMutexAndCondAttr(pthread_mutex_t& mutex, pthread_mutexattr_t& attr, pthread_cond_t& cond_var) {
    if (pthread_mutexattr_init(&attr) != 0) {
        perror("pthread_mutex_attr_init() error");
        exit(1);
    } 
    
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        perror("pthread_mutexattr_settype() error");
        exit(1);
    }
    
    if (pthread_mutex_init(&mutex, &attr) != 0) {
        perror("pthread_mutex_init() error");
        exit(1);
    }

    if (pthread_cond_init(&cond_var, NULL) != 0) {
        perror("pthread_cond_init() error");
        exit(1);
    }

}


void initMutexAndAttr(pthread_mutex_t& mutex, pthread_mutexattr_t& attr) {
    if (pthread_mutexattr_init(&attr) != 0) {
        perror("pthread_mutex_attr_init() error");
        exit(1);
    } 
    
    if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
        perror("pthread_mutexattr_settype() error");
        exit(1);
    }
    
    if (pthread_mutex_init(&mutex, &attr) != 0) {
        perror("pthread_mutex_init() error");
        exit(1);
    }
}

template <typename T>
void* assigner_function(void* args) {
    for (;;)  {
        reinterpret_cast<ThreadPool<T>*>(args)->assign_job();
        usleep(100000);
    }

    return NULL;
}



template <typename T>
ThreadPool<T>::ThreadPool(uint16_t num_workers, workerFunction wf) {
    if (wf == NULL) {
        perror("workerFunction wf was null");
        exit(1);
    }
    this->wf = wf;

    pthread_mutexattr_t jq_attr;
    initMutexAndAttr(job_queue_lock, jq_attr);
   
    uint16_t thread_count = std::min((uint16_t) MAX_THREADS, num_workers);
    jobAssignments.resize(thread_count); //Expand to number of threads to be allocated

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


        pthread_mutex_t* mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
        if (mutex == NULL) {
            perror("Malloc issue for mutex");
            exit(1);
        }

        pthread_cond_t* cond = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
        if (mutex == NULL) {
            perror("Malloc issue for pthread_cond_t");
            exit(1);
        }

        pthread_mutexattr_t attr;
        initMutexAndCondAttr(*mutex, attr, *cond);

        thread_mutexes.push_back(mutex);
        thread_cond_vars.push_back(cond);
        thread_dispatch_indicators.push_back(false);

        pthread_create(thread_ID, NULL, &ThreadPool::pthread_worker_wrapper, ta);
        threads.push_back(thread_ID);
    }

    assigner_thread = (pthread_t*) malloc(sizeof(pthread_t));
    if (assigner_thread == NULL) {
            perror("malloc error for pthread_t");
            exit(1);
    }

    pthread_create(assigner_thread, NULL, assigner_function<ChordJob>, this);

    fprintf(stderr, "Finished ThreadPool() \n");
}

template <typename T>
void ThreadPool<T>::add_job(T job) {
    fprintf(stderr, "Add_Job() \n");

    pthread_mutex_lock(&job_queue_lock);
    jobs.push(job);
    pthread_mutex_unlock(&job_queue_lock);
}

template <typename T>
void ThreadPool<T>::assign_job() {
    pthread_mutex_lock(&job_queue_lock);
    if (jobs.empty()) {
        pthread_mutex_unlock(&job_queue_lock);
        return;
    }

    for (int i = 0; i < threads.size(); i++) {
        bool assignedJob = false;
        pthread_mutex_lock(thread_mutexes.at(i)); //Prevent access to dispatch indicator
        if (!thread_dispatch_indicators.at(i)) { //Then we can assign job to this thread
            T job = jobs.front();
            jobs.pop();
            jobAssignments.at(i) = job;
            thread_dispatch_indicators.at(i) = true;
            assignedJob = true;
        }
        
        pthread_mutex_unlock(thread_mutexes.at(i)); //Prevent access to dispatch indicator
        if (assignedJob) {
            pthread_cond_signal(thread_cond_vars.at(i));
        }
    }

    pthread_mutex_unlock(&job_queue_lock);
}


template <typename T>
ThreadPool<T>::~ThreadPool() {
    for (pthread_t* t : threads) {
        if (pthread_cancel(*t) != 0) {
            perror("pthread_cancel() faced an error, continuing to cancel other threads");
        }
        //free dynamically allocated stuff
        free(t);
    }

    //Destroy the mutexes in the vector!! TODO and free them as well!
    for (auto m : thread_mutexes) {
        pthread_mutex_destroy(m);
        free(m);
    }

    for (auto c : thread_cond_vars) {
        pthread_cond_destroy(c);
        free(c);
    }

    if (pthread_cancel(*assigner_thread) != 0) {
        perror("pthread_cancel() faced an error, continuing to cancel other threads");
    }
    free(assigner_thread);


}


template <typename T>
void* ThreadPool<T>::worker_wrapper(void* args) {
    int sequential_thread_id = (int) (intptr_t) args;
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    for (;;) {
        pthread_mutex_lock(thread_mutexes.at(sequential_thread_id)); //Prevent access to dispatch indicators
        if (!thread_dispatch_indicators.at(sequential_thread_id)) {
            //Wait for a job to be assigned
            pthread_cond_wait(thread_cond_vars.at(sequential_thread_id), thread_mutexes.at(sequential_thread_id));
        }

        //Job has been assigned so now retrieve the assigned job
        T job_to_execute = jobAssignments.at(sequential_thread_id);
        pthread_mutex_unlock(thread_mutexes.at(sequential_thread_id));

        // fprintf(stderr, "THREAD: %i taking job # %i from queue. Queue Size Pre: %lu \n", (int) (intptr_t) args, ((ChordJob) job_to_execute).val, jobs.size());

        //Execute the assigned job
        this->wf(job_to_execute, args);


        //Indicate to the boss that this worker thread is done executing its assigned job
        pthread_mutex_lock(thread_mutexes.at(sequential_thread_id));
        thread_dispatch_indicators.at(sequential_thread_id) = false;
        pthread_mutex_unlock(thread_mutexes.at(sequential_thread_id));

    }
    return NULL;
}

template class ThreadPool<ChordJob>;