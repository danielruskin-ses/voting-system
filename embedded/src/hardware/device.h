/* device.h */

#ifndef HARDWARE_DEVICE_H_
#define HARDWARE_DEVICE_H_

#include "battery.h"
#include "display.h"
#include "ethernetadapter.h"
#include "gpio.h"
#include "indicator.h"
#include "keypad.h"
#include "printer.h"
#include "wifiadapter.h"

#include <memory>
#include <string>


class Device
{
private:
	Battery _battery;
	Display _display;
	EthernetAdapter _ethernetAdapter;
	GPIO _gpio;
	Indicator _powerIndicator;
	Keypad _keypad;
	Printer _printer;
	WiFiAdapter _wifiAdapter;

	bool _running;
public:
	typedef std::shared_ptr<Device> Ptr;
	
	Device() {}
	~Device() {}

	void init();
	void update();
	void shutdown();
	std::string scanString();

	inline bool isRunning() const { return _running; }
};


#endif /* HARDWARE_DEVICE_H_ */
