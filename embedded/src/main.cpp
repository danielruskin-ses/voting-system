/* main.cpp */

#include "system.h"

#include <memory>

#include <cstdlib>
#include <ctime>


using namespace std;

int main(int argc, char **argv)
{
	System::Ptr system = make_shared<System>();
	system->start();

	return 0;
};

