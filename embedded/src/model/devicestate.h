/*
 * devicestate.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef MODEL_DEVICESTATE_H_
#define MODEL_DEVICESTATE_H_

enum class DeviceState
{
	Uninitialized, Setup, Authenticate, Vote, Receipt
};

#endif /* MODEL_DEVICESTATE_H_ */
