/*
 * authenticationcontroller.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef CONTROLLER_AUTHENTICATECONTROLLER_H_
#define CONTROLLER_AUTHENTICATECONTROLLER_H_

#include "controller.h"

class AuthenticateController : public Controller
{
public:
	AuthenticateController(const DeviceModel& device, const Interface& interface);
	virtual ~AuthenticateController();

	virtual void handleInput();
};

#endif /* CONTROLLER_AUTHENTICATECONTROLLER_H_ */
