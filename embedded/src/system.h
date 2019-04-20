/* system.h */

#ifndef SYSTEM_H_
#define SYSTEM_H_


#include "election/electionsystem.h"
#include "hardware/device.h"
#include "hardware/keypad.h"
#include "systemstate.h"

#include <memory>


class SystemState;

class System : public Keypad::Listener, public std::enable_shared_from_this<System>
{
private:
	Device::Ptr _device;
	ElectionSystem::Ptr _electionSystem;

	SystemState::Ptr _state;
	bool _running;
public:
	typedef std::shared_ptr<System> Ptr;

	System();
	virtual ~System() {}

	void start();
	void stop();

	virtual void notify(Keypad::Event::Ptr event);

	inline void setState(SystemState::Ptr state) { _state = state; }
	inline bool isRunning() const { return _running; }
	inline Device::Ptr getDevice() const { return _device; }
	inline ElectionSystem::Ptr getElectionSystem() const { return _electionSystem; }
};


#endif /* SYSTEM_H_ */
