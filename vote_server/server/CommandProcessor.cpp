#include "CommandProcessor.h"
#include <iostream>

int processCommand(int sock, char* msg, int len) {
        // Begin by parsing command (byte 0 until first EOF)
        int cmdStart = 0;
        int cmdEnd = 0;
        while(cmdEnd < len - 1) {
                if(msg[
        }
}
