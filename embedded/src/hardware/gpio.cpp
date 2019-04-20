/* gpio.cpp */

#include "gpio.h"

#include <cstdlib>


using namespace std;

bool GPIO::getInput(int pin) const
{
	int r = rand();
	if (r > RAND_MAX/2) {
		return true;
	} else {
		return false;
	}
}


void GPIO::setOutput(int pin, bool state) const
{

}

