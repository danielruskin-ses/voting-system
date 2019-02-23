#include <iostream>
#include "logger/Logger.h"
#include "server/Server.h"

int main(int argc, char** argv) {
        Logger logger(std::cout, std::cerr);

        logger.info("Foo Bar Baz");
}
