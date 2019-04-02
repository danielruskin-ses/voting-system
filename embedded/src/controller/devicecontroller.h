/*
 * devicecontroller.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef CONTROLLER_DEVICECONTROLLER_H_
#define CONTROLLER_DEVICECONTROLLER_H_

#include "authenticatecontroller.h"
#include "controller.h"

#include "setupcontroller.h"
#include "votecontroller.h"
#include "receiptcontroller.h"

class DeviceController : public Controller
{
private:
	SetupController _setupController;
	AuthenticateController _authController;
	VoteController _voteController;
	ReceiptController _receiptController;
public:
	DeviceController(const DeviceModel& device, const Interface& interface);
	virtual ~DeviceController();

	virtual void handleInput();
};

#endif /* CONTROLLER_DEVICECONTROLLER_H_ */
