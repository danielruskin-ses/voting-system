#include "Sockets.h"

#define SOCKET_LOOP_TIMEOUT_SEC 5

int checkSocketForData(int sockfd) {
        struct timeval tv;
        tv.tv_sec = SOCKET_LOOP_TIMEOUT_SEC; 
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        
        return select(sockfd + 1, &rfds, NULL, NULL, &tv);
}

int socketRecv(int sockfd, char* buf, int bufSize) {
        struct timeval tv;
        tv.tv_sec = SOCKET_LOOP_TIMEOUT_SEC; 
        fd_set rfds;
        FD_ZERO(&rfds);
        FD_SET(sockfd, &rfds);
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        // Read until we get the full msg, or we get an error
        // Note that this blocks indefinitely if a client sends a malformed message.
        // This is OK, because clients can only block their own connections,
        // and connections timeout after K seconds.
        int atByte = 0;
        while(atByte < bufSize) {
                int res = recv(sockfd, buf + atByte, bufSize - atByte, 0);
                if(res < 0) {
                        return res;
                }
                atByte += res;
        }

        return 0;
}
