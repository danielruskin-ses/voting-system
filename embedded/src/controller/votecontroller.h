/*
 * votecontroller.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef CONTROLLER_VOTECONTROLLER_H_
#define CONTROLLER_VOTECONTROLLER_H_

#include "controller.h"

class VoteController : public Controller
{
public:
	VoteController(const DeviceModel& device, const Interface& interface);
	virtual ~VoteController();

	virtual void handleInput();
};

#endif /* CONTROLLER_VOTECONTROLLER_H_ */
