/* device.cpp */

#include "device.h"


using namespace std;

void Device::init()
{
	_running = true;
}

void Device::update()
{
	_keypad.poll(_gpio);
}

void Device::shutdown()
{
	_running = false;
}

