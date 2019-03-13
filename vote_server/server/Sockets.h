#pragma once

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>

int checkSocketForData(int sockfd);
int socketRecv(int sockfd, uint8_t* buf, int bufSize);
int socketSend(int sockfd, uint8_t* buf, int bufSize);
