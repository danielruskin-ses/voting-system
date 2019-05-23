/* system.h */

#ifndef SYSTEM_H_
#define SYSTEM_H_


#include "config.h"
#include "systemstate.h"
#include "hardware/hardware.h"
#include "election/electionsystem.h"
#include "network/connection.h"

#include <memory>


class SystemState;

class System
{
private:
	SystemState::Ptr _state;		// System state
	bool _running;
public:
	typedef std::shared_ptr<System> Ptr;
	
	Config _config;								// System configuration
	Hardware _hardware;							// Representation of device hardware
	std::unique_ptr<Connection> _connection;	// Connection to vote server
	ElectionSystem _electionSystem;				// Representation of election

	System() {}
	virtual ~System() {}

	void start();	// Initialize the device
	void stop();	// Shutdown the device
	void setState(SystemState::Ptr state);

	inline bool isRunning() const { return _running; }
};


#endif /* SYSTEM_H_ */
