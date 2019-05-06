/* device.cpp */

#include "device.h"


using namespace std;

void Device::init()
{
	_running = true;
}

void Device::update()
{

}

void Device::shutdown()
{
	_running = false;
}

