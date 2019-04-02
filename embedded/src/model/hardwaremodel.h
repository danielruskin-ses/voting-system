/*
 * hardware.h
 *
 *  Created on: Apr 1, 2019
 *      Author: jonathandavis
 */

#ifndef MODEL_HARDWAREMODEL_H_
#define MODEL_HARDWAREMODEL_H_

#include "../hardware/battery.h"
#include "../hardware/display.h"
#include "../hardware/ethernetadapter.h"
#include "../hardware/indicator.h"
#include "../hardware/keypad.h"
#include "../hardware/printer.h"
#include "../hardware/wifiadapter.h"

class HardwareModel
{
private:
	Battery _battery;
	Display _display;
	EthernetAdapter _ethernetAdapter;
	Indicator _powerIndicator;
	Keypad _keypad;
	Printer _printer;
	WiFiAdapter _wifiAdapter;
public:
	HardwareModel();
	virtual ~HardwareModel();

	void init();

	inline const Battery& getBattery() const { return _battery; }
	inline const Display& getDisplay() const { return _display; }
	inline const EthernetAdapter& getEthernetAdapter() const { return _ethernetAdapter; }
	inline const Indicator& getPowerIndicator() const { return _powerIndicator; }
	inline const Keypad& getKeypad() const { return _keypad; }
	inline const Printer& getPrinter() const { return _printer; }
	inline const WiFiAdapter& getWiFiAdapter() const { return _wifiAdapter; }
};

#endif /* MODEL_HARDWAREMODEL_H_ */
