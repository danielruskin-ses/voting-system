/*
 * devicemodel.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef MODEL_DEVICEMODEL_H_
#define MODEL_DEVICEMODEL_H_

#include "devicestate.h"
#include "../model/hardwaremodel.h"
#include "../model/electionmodel.h"

class DeviceModel
{
private:
	HardwareModel _hardwareModel;
	ElectionModel _electionModel;
	DeviceState _deviceState;
	bool _running;
public:
	DeviceModel();
	virtual ~DeviceModel();

	void start();
	void stop();

	inline const HardwareModel& getHardware() const { return _hardwareModel; }
	inline const ElectionModel& getElection() const { return _electionModel; }
	inline DeviceState getDeviceState() const { return _deviceState; }
	inline bool isRunning() const { return _running; }
};

#endif /* MODEL_DEVICEMODEL_H_ */
