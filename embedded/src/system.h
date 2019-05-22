/* system.h */

#ifndef SYSTEM_H_
#define SYSTEM_H_


#include "election/electionsystem.h"
#include "hardware/device.h"
#include "hardware/keypad.h"
#include "systemstate.h"

#include <memory>


class SystemState;

class System
{
private:
	Device _device;
	SystemState::Ptr _state;
	ElectionSystem _electionSystem;
	bool _running;
public:
	typedef std::shared_ptr<System> Ptr;

	System() {}
	virtual ~System() {}

	void start();
	void stop();
	void setState(SystemState::Ptr state);

	inline bool isRunning() const { return _running; }
	inline Device& getDevice() { return _device; }
	inline ElectionSystem& getElectionSystem() { return _electionSystem; }
};


#endif /* SYSTEM_H_ */
