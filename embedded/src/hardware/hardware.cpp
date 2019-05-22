/* hardware.cpp */

#include "hardware.h"

#include <iostream>


using namespace std;

void Hardware::init()
{
	_running = true;
}

void Hardware::update()
{

}

void Hardware::shutdown()
{
	_running = false;
}
