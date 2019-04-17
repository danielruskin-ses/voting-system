/* system.h */

#ifndef SYSTEM_H_
#define SYSTEM_H_


#include "hardware/device.h"
#include "election/electionsystem.h"


class System
{
private:
	Device::Ptr _device;
	ElectionSystem::Ptr _electionSystem;
public:
	System();
	~System() {}

	void run();
};


#endif /* SYSTEM_H_ */
