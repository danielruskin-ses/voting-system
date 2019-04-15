#include "Sockets.h"
#include "shared_c/Time.h"

int checkSocketForData(int sockfd, int timeoutSec) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        // Max wait 5 seconds
        struct timeval tv = {timeoutSec, 0};
        return select(sockfd + 1, &readfds, NULL, NULL, &tv);
}

int socketSend(int sockfd, BYTE_T* buf, int bufSize, int timeoutSec) {
        int time = getCurrentTime();

        int atByte = 0;
        while(atByte < bufSize) {
                int res = send(sockfd, buf + atByte, bufSize - atByte, 0);
                if(res < 0) {
                        return res;
                }
                atByte += res;

                if(getCurrentTime() - time > timeoutSec) {
                        return -1;
                }
        }

        return 0;
}

int socketRecv(int sockfd, BYTE_T* buf, int bufSize, int timeoutSec) {
        int time = getCurrentTime();

        // Read until we get the full msg, or we get an error/timeout
        // Note that this blocks indefinitely if a client sends a malformed message.
        int atByte = 0;
        while(atByte < bufSize) {
                int res = recv(sockfd, buf + atByte, bufSize - atByte, 0);
                if(res < 0) {
                        return res;
                }
                atByte += res;

                if(getCurrentTime() - time > timeoutSec) {
                        return -1;
                }
        }

        return 0;
}
