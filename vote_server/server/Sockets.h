#pragma once

#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>

int checkSocketForData(int sockfd);
int socketRecv(int sockfd, char* buf, int bufSize, int loopMax);
