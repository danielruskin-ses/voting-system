/* hardware.h */

#ifndef HARDWARE_HARDWARE_H_
#define HARDWARE_HARDWARE_H_

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


class Hardware
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
	typedef std::shared_ptr<Hardware> Ptr;
	
	Hardware() {}
	~Hardware() {}

	void init();
	void update();
	void shutdown();

	inline bool isRunning() const { return _running; }
};


#endif /* HARDWARE_HARDWARE_H_ */
