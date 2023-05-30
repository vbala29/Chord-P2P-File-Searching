/**
 * Developer: Vikram Bala
 * Contact: vikrambala2002@gmail.com
 * Copyright © 2023 Vikram Bala
 */
#pragma once

#include "penn-chord-message.h"
#include "ipv4.hpp"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void sendTo(PennChordMessage message, int port, Ipv4Address ip);