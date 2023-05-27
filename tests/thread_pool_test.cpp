#include "../include/thread_pool.hpp"
#include <future>

typedef struct {
    int val = 0;
} ChordJob;

void chordFunction(ChordJob cj, void* args) {
    fprintf(stdout, "THREAD: %i \n \t The val of this chord job is %i \n", args, cj.val);
}

int main(int argc, char* argv) {
    ThreadPool<ChordJob>::workerFunction wf = chordFunction;
    ThreadPool tp(5, wf);

    for (int i = 0; i < 10; i++) {
        ChordJob cj = {i};
        std::async(std::launch::async, &ThreadPool<ChordJob>::add_job, tp, cj);
    }
}


