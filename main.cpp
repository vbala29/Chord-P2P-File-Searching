// #include "tests/thread_pool_test.hpp"
// #include <cstdlib>

#include "src/penn-chord.h"
#include <string>
#include <map>

// First arg is Ipv4 of this node, 2nd arg is its node Id
int main(int argc, char** argv) {
    if (argc < 2) {
        perror("Too few arguments");
        exit(1);
    }

    PennChord pc{};

    std::map<uint32_t, Ipv4Address> m_nodeAddressMap = {
        {1, Ipv4Address(1)},
        {2, Ipv4Address(2)},
        {3, Ipv4Address(3)},
        {4, Ipv4Address(4)},
    };

    std::map<Ipv4Address, uint32_t> m_addressNodeMap = {
        {Ipv4Address(1), 1},
        {Ipv4Address(2), 2},
        {Ipv4Address(3), 3},
        {Ipv4Address(4), 4},
    };

    pc.StartApplication(m_nodeAddressMap, m_addressNodeMap, Ipv4Address(std::stoi(argv[1])), argv[2]);
}