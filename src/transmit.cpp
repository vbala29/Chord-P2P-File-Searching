/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "transmit.h"

void sendTo(PennChordMessage message, int port, Ipv4Address ip) {
    int sockfd, portno;
    struct sockaddr_in node_addr;

    while(sockfd < 0) {
        sockfd = socket(AF_INET, SOCK_STREAM, PF_INET); //internetwork, TCP/IP, IPv4
        if (sockfd < 0) {
            perror("Error opening socket");
        }
    }

    bzero((char*) &node_addr, sizeof(node_addr));
    portno = port;

    node_addr.sin_family = AF_INET; //Ipv4 protocol
    inet_aton(ip.Ipv4ToString().c_str(), &node_addr.sin_addr); 
    node_addr.sin_port = htons(portno); //convert to network byte order

    while(connect(sockfd, (struct sockaddr *) &node_addr, sizeof(node_addr)) < 0) {
        perror("Error on connecting");
    }

    Buffer b{};
    Buffer::Iterator it = b.Begin();
    message.Serialize(it);
    uint8_t* buf = (uint8_t*) malloc(sizeof(uint8_t) * b.GetSerializedSize());
    if (b.Serialize(buf, b.GetSerializedSize()) == 0) {
        perror("Buffer not large enough");
    }

    if (send(sockfd, buf, (size_t) b.GetSerializedSize(), 0) < 0) {
        perror("Error with send(2) call");
    } 

    free(buf);
    close(sockfd); //Close file descriptor/delete from per process table
}