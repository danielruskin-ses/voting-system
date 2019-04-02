/*
 * setupcontroller.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef CONTROLLER_SETUPCONTROLLER_H_
#define CONTROLLER_SETUPCONTROLLER_H_

#include "controller.h"

class SetupController : public Controller
{
public:
	SetupController(const DeviceModel& device, const Interface& interface);
	virtual ~SetupController();

	virtual void handleInput();
};

#endif /* CONTROLLER_SETUPCONTROLLER_H_ */
