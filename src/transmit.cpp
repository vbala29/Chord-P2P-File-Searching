/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "transmit.h"

void sendTo(PennChordMessage message, int port, Ipv4Address ip) {
    int sockfd, portno;
    struct sockaddr_in node_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //internetwork, TCP/IP, IPv4
    if (sockfd < 0) {
        perror("Error opening socket");
        exit(1);
    }


    bzero((char*) &node_addr, sizeof(node_addr));
    portno = port;

    node_addr.sin_family = AF_INET; //Ipv4 protocol
    inet_aton(ip.Ipv4ToString().c_str(), &node_addr.sin_addr); 
    node_addr.sin_port = htons(portno); //convert to network byte order
    std::cout << "JHEREWEEE: " << ip.Ipv4ToString() << std::flush;

    if (connect(sockfd, (struct sockaddr *) &node_addr, sizeof(node_addr)) < 0) {
        std::cout << "Sockfd: " << sockfd << std::endl;
        perror("Error on connecting");
        exit(1);
    }
    
    std::cout << "1JHEREWEEE" << std::flush;

    Buffer b{};
    Buffer::Iterator it = b.Begin();
    message.Serialize(it);
    uint8_t* buf = (uint8_t*) malloc(sizeof(uint8_t) * b.GetSerializedSize());
    if (b.Serialize(buf, b.GetSerializedSize()) == 0) {
        perror("Buffer not large enough");
    }

    std::cout << "2JHEREWEEE" << std::flush;
 
    if (send(sockfd, buf, (size_t) b.GetSerializedSize(), 0) < 0) {
        perror("Error with send(2) call");
    } 

    std::cout << "3JH123EREWEEE" << std::flush;

    free(buf);
    close(sockfd); //Close file descriptor/delete from per process table
}