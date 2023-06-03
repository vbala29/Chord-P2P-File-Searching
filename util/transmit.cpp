/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#include "../include/transmit.h"

void sendTo(PennChordMessage message, int port, Ipv4Address ip) {
    int sockfd, portno;
    struct sockaddr_in node_addr;

    // std::cout << "Sending message: " << message.GetMessageType() << ", on port: " << port << ", to IP: " << 
    //         ip.Ipv4ToString() << std::endl << std::flush;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //internetwork, TCP/IP, default protocol
    if (sockfd < 0) {
        close(sockfd);
        perror("Error opening socket");
        exit(1);
    }



   //bzero((char*) &node_addr, sizeof(node_addr));
    portno = port;

    node_addr.sin_family = AF_INET; //TCP protocol
    node_addr.sin_port = htons(portno); //convert to network byte order

    if (inet_pton(AF_INET, ip.Ipv4ToString().c_str(), &node_addr.sin_addr) <= 0) {
        close(sockfd);
        perror("invalid address/adress not supported");
        exit(1);
    }


    if (connect(sockfd, (struct sockaddr *) &node_addr, sizeof(node_addr)) < 0) {
        close(sockfd);
        std::cout << "Sockfd: " << sockfd << std::endl;
        perror("Error on connecting");
        exit(1);
    }
    

    BufferV2 b{};
    // std::cout << "Size of buffer rn : " << b.GetSerializedSize() << ", size needed: " << message.GetSerializedSize() << std::endl << std::flush;
    
    message.Serialize(b);
    uint8_t* buf = (uint8_t*) calloc(sizeof(uint8_t) * message.GetSerializedSize(), 1);
    if (b.Serialize(buf, message.GetSerializedSize()) == 0) {
        close(sockfd);
        perror("Buffer not large enough");
    }

    // std::cout << "Size of buffer now: " << b.GetSerializedSize() << std::endl << std::flush;
    // for (int i = 0; i < b.GetSerializedSize(); i++) {
    //     std::cout << unsigned(buf[i]) << ", ";
    // } 
    // std::cout << std::endl << std::flush;
 
    if (send(sockfd, buf, (size_t) b.GetSerializedSize(), 0) < 0) {
        close(sockfd);
        perror("Error with send(2) call");
    } 

    free(buf);
    close(sockfd); //Close file descriptor/delete from per process table
}


void sendTo(PennSearchMessage message, int port, Ipv4Address ip) {
    int sockfd, portno;
    struct sockaddr_in node_addr;

    // std::cout << "Sending message: " << message.GetMessageType() << ", on port: " << port << ", to IP: " << 
    //         ip.Ipv4ToString() << std::endl << std::flush;

    sockfd = socket(AF_INET, SOCK_STREAM, 0); //internetwork, TCP/IP, default protocol
    if (sockfd < 0) {
        close(sockfd);
        perror("Error opening socket");
        exit(1);
    }



   //bzero((char*) &node_addr, sizeof(node_addr));
    portno = port;

    node_addr.sin_family = AF_INET; //TCP protocol
    node_addr.sin_port = htons(portno); //convert to network byte order

    if (inet_pton(AF_INET, ip.Ipv4ToString().c_str(), &node_addr.sin_addr) <= 0) {
        close(sockfd);
        perror("invalid address/adress not supported");
        exit(1);
    }


    if (connect(sockfd, (struct sockaddr *) &node_addr, sizeof(node_addr)) < 0) {
        close(sockfd);
        std::cout << "Sockfd: " << sockfd << std::endl;
        perror("Error on connecting");
        exit(1);
    }
    

    BufferV2 b{};
    message.Serialize(b);
    uint8_t* buf = (uint8_t*) calloc(sizeof(uint8_t) * message.GetSerializedSize(), 1);
    if (b.Serialize(buf, message.GetSerializedSize()) == 0) {
        close(sockfd);
        perror("Buffer not large enough");
        exit(1);
    }

    // std::cout << "Size of buffer now: " << b.GetSerializedSize() << std::endl << std::flush;
    // for (int i = 0; i < b.GetSerializedSize(); i++) {
    //     std::cout << unsigned(buf[i]) << ", ";
    // } 
    // std::cout << std::endl << std::flush;
 
    if (send(sockfd, buf, (size_t) b.GetSerializedSize(), 0) < 0) {
        close(sockfd);
        perror("Error with send(2) call");
        exit(1);
    } 

    free(buf);
    close(sockfd); //Close file descriptor/delete from per process table
}