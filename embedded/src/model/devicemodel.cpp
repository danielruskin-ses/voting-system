/*
 * devicemodel.cpp
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#include "devicemodel.h"

DeviceModel::DeviceModel() :
	_deviceState(DeviceState::Uninitialized),
	_running(false)
{

}

DeviceModel::~DeviceModel()
{

}

void DeviceModel::start()
{
	_hardwareModel.init();

	_deviceState = DeviceState::Setup;
	_running = true;
}

void DeviceModel::stop()
{
	_running = false;
}
