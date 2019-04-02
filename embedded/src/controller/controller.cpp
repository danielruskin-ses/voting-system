/*
 * controller.cpp
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#include "controller.h"

Controller::Controller(const DeviceModel& device, const Interface& interface):
	_device(device),
	_interface(interface)
{

}

Controller::~Controller()
{

}
