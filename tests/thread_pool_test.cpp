#include "../lib/thread_pool.hpp"
#include <future>
#include "../src/chord_job.hpp"

void chordFunction(ChordJob cj, void* args) {
    fprintf(stdout, "THREAD: %i \n \t The val of this chord job is %i \n", (int) (intptr_t) args, cj.val);
}

void tp_test() {
    ThreadPool<ChordJob>::workerFunction wf = chordFunction;
    ThreadPool<ChordJob> tp(5, wf);

    for (int i = 0; i < 10; i++) {
        ChordJob cj = {i};
        std::async(std::launch::async, &ThreadPool<ChordJob>::add_job, tp, cj);
    }
}


