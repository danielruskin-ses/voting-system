#include "CommandProcessor.h"
#include <iostream>

int processCommand(char* msg, int len) {
        std::cout << "got to 1: ";
        for(int i = 0; i < len; i++) {
                std::cout << msg[i];
        }
        std::cout << std::endl;
}
