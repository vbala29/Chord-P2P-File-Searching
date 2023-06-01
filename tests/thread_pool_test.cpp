#include "../lib/thread_pool.hpp"
#include <future>
#include "../src/chord_job.hpp"
#include <unistd.h>

void chordFunction(ChordJob cj, void* args) {
    fprintf(stderr, "THREAD: %i. The val of this chord job is %i \n", (int) (intptr_t) args, cj.val);
}

void tp_test() {
    fprintf(stderr, "Beginning Thread Pool Test \n--------------------------------- \n");
    ThreadPool<ChordJob>::workerFunction wf = chordFunction;
    ThreadPool<ChordJob> tp(10, wf);

    for (int i = 0; i < 200; i++) {
        if (i % 20 == 0) usleep(20000);
        ChordJob cj = {i};
        tp.add_job(cj);
    }

    sleep(2);    

}


