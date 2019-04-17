/* system.cpp */

#include "system.h"


using namespace std;

System::System() : _device(make_shared<Device>()), _electionSystem(make_shared<ElectionSystem>()) {}

void System::run()
{
	_device->init();

	while (_device->isRunning()) {
		_device->update();
		_electionSystem->update();
	}

	_device->shutdown();
};

