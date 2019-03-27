#pragma once

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>

#include "shared_c/Definitions.h"

int checkSocketForData(int sockfd);
int socketRecv(int sockfd, BYTE_T* buf, int bufSize);
int socketSend(int sockfd, BYTE_T* buf, int bufSize);
