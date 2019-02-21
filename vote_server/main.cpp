#include "stdio.h"
#include "Logger.h"

int main(int argc, char** argv) {
        Logger logger(stdout, stderr);

        logger.info("Foo Bar Baz");
}
