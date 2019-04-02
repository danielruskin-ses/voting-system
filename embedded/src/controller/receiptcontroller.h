/*
 * receiptcontroller.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef CONTROLLER_RECEIPTCONTROLLER_H_
#define CONTROLLER_RECEIPTCONTROLLER_H_

#include "controller.h"

class ReceiptController : public Controller
{
public:
	ReceiptController(const DeviceModel& device, const Interface& interface);
	virtual ~ReceiptController();

	virtual void handleInput();
};

#endif /* CONTROLLER_RECEIPTCONTROLLER_H_ */
