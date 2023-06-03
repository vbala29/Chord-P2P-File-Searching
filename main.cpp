// #include "tests/thread_pool_test.hpp"
// #include <cstdlib>

#include <string>
#include <pthread.h>
#include <map>
#include "include/penn-chord.h"


// First arg is Ipv4 of this node, 2nd arg is its node Id
int main(int argc, char** argv) {
    if (argc < 2) {
        perror("Too few arguments");
        exit(1);
    }

    PennChord pc;

    //These maps are used to contain the IP addresses of each POSSIBLE node in the ring.
    std::map<uint32_t, Ipv4Address> m_nodeAddressMap = {
        {1, Ipv4Address(192, 168, 86, 216)},
        {2, Ipv4Address(192, 168, 86, 230)},
        {3, Ipv4Address(192, 168, 86, 84)},
        {4, Ipv4Address(192, 168, 86, 232)},
    };

    std::map<Ipv4Address, uint32_t> m_addressNodeMap = {
        {Ipv4Address(192, 168, 86, 216), 1},
        {Ipv4Address(192, 168, 86, 230), 2},
        {Ipv4Address(192, 168, 86, 84), 3},
        {Ipv4Address(192, 168, 86, 232), 4},
    };

    std::cout << "ARGV1: " << std::string(argv[1]) << ", ARGV2: " << std::string(argv[2]) << std::endl;
    pc.StartApplication(m_nodeAddressMap, m_addressNodeMap, Ipv4Address(std::stoi(argv[1])), argv[2]);

    while(1);
}