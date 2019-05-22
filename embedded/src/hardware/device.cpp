/* device.cpp */

#include "device.h"

#include <iostream>


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

string Device::scanString()
{
	string s;
	cin >> s;
	return s;
}

