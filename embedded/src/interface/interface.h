/*
 * interface.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef INTERFACE_INTERFACE_H_
#define INTERFACE_INTERFACE_H_

#include "uninitializedscreen.h"
#include "setupscreen.h"
#include "authenticatescreen.h"
#include "votescreen.h"
#include "receiptscreen.h"

#include "../model/devicemodel.h"

class Interface
{
private:
	UninitializedScreen _uninitializedScreen;
	SetupScreen _setupScreen;
	AuthenticateScreen _authenticateScreen;
	VoteScreen _voteScreen;
	ReceiptScreen _receiptScreen;

	DeviceModel _device;
public:
	Interface(const DeviceModel& device);
	virtual ~Interface();
};

#endif /* INTERFACE_INTERFACE_H_ */
