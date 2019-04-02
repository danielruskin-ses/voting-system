/*
 * controller.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef CONTROLLER_CONTROLLER_H_
#define CONTROLLER_CONTROLLER_H_

#include "../model/devicemodel.h"
#include "../interface/interface.h"

class Controller
{
protected:
	DeviceModel _device;
	Interface _interface;
public:
	Controller(const DeviceModel& device, const Interface& interface);
	virtual ~Controller();

	virtual void handleInput() = 0;
};

#endif /* CONTROLLER_CONTROLLER_H_ */
