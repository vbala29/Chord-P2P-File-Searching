/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */

#ifndef TRANSMIT_H
#define TRANSMIT_H

#include "penn-chord-message.h"
#include "ipv4.hpp"
#include "BufferV2.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>

void sendTo(PennChordMessage message, int port, Ipv4Address ip);

#endif