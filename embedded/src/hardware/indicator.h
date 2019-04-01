/*
 * indicator.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 *
 *  Representation of an indicator light
 */

#ifndef SRC_HARDWARE_INDICATOR_H_
#define SRC_HARDWARE_INDICATOR_H_

class Indicator
{
private:
	bool _powered;
public:
	Indicator();
	virtual ~Indicator();

	void turnOn();
	void turnOff();
	inline bool isPowered() const { return _powered; }
};

#endif /* SRC_HARDWARE_INDICATOR_H_ */
