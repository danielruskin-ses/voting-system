/* device.cpp */

#include "device.h"

#include <iostream>


using namespace std;

void Device::init()
{
	cout << "Initializing device" << endl;

	_running = true;
}

void Device::update()
{
	cout << "Updating device" << endl;
}

void Device::shutdown()
{
	cout << "Shutting down device" << endl;
}

