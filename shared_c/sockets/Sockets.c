#include "Sockets.h"

int checkSocketForData(int sockfd) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        return select(sockfd + 1, &readfds, NULL, NULL, NULL);
}

int socketSend(int sockfd, BYTE_T* buf, int bufSize) {
        int atByte = 0;
        while(atByte < bufSize) {
                int res = send(sockfd, buf + atByte, bufSize - atByte, 0);
                if(res < 0) {
                        return res;
                }
                atByte += res;
        }

        return 0;
}

int socketRecv(int sockfd, BYTE_T* buf, int bufSize) {
        // Read until we get the full msg, or we get an error
        // Note that this blocks indefinitely if a client sends a malformed message.
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
