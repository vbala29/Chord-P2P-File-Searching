// #include "tests/thread_pool_test.hpp"
// #include <cstdlib>

#include <string>
#include <pthread.h>
#include <map>
#include "src/penn-chord.h"


// First arg is Ipv4 of this node, 2nd arg is its node Id
int main(int argc, char** argv) {
    if (argc < 2) {
        perror("Too few arguments");
        exit(1);
    }

    PennChord pc;

    std::map<uint32_t, Ipv4Address> m_nodeAddressMap = {
        {1, Ipv4Address(192, 168, 86, 216)},
        {2, Ipv4Address(192, 168, 200, 23)},
        {3, Ipv4Address(127, 0, 0, 3)},
        {4, Ipv4Address(127, 0, 0, 4)},
    };

    std::map<Ipv4Address, uint32_t> m_addressNodeMap = {
        {Ipv4Address(192, 168, 86, 216), 1},
        {Ipv4Address(192, 168, 200, 23), 2},
        {Ipv4Address(127, 0, 0, 3), 3},
        {Ipv4Address(127, 0, 0, 4), 4},
    };

    pc.StartApplication(m_nodeAddressMap, m_addressNodeMap, Ipv4Address(std::stoi(argv[1])), argv[2]);

    while(1);

    // for (std::pair<std::string, pthread_t> p: threadMap) {
    //     pthread_cancel(p.second);
    // }
}