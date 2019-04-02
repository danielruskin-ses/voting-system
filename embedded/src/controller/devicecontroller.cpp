/*
 * devicecontroller.cpp
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#include "devicecontroller.h"

DeviceController::DeviceController(const DeviceModel& device, const Interface& interface) :
	Controller(device, interface),
	_setupController(device, interface),
	_authController(device, interface),
	_voteController(device, interface),
	_receiptController(device, interface)
{

}

DeviceController::~DeviceController()
{

}

void DeviceController::handleInput()
{
	switch (_device.getDeviceState()) {
	case DeviceState::Uninitialized:
		break;
	case DeviceState::Setup:
		_setupController.handleInput();
		break;
	case DeviceState::Authenticate:
		_authController.handleInput();
		break;
	case DeviceState::Vote:
		_voteController.handleInput();
		break;
	case DeviceState::Receipt:
		_receiptController.handleInput();
		break;
	}
}
