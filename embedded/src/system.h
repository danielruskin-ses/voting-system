/* system.h */

#ifndef SYSTEM_H_
#define SYSTEM_H_


#include "election/electionsystem.h"
#include "hardware/hardware.h"
#include "systemstate.h"
#include "config.h"

#include <memory>


class SystemState;

class System
{
private:
	Hardware _hardware;
	SystemState::Ptr _state;
	ElectionSystem _electionSystem;
	bool _running;
	
	int _sock;
	Config _config;
public:
	typedef std::shared_ptr<System> Ptr;

	System() {}
	virtual ~System() {}

	void start();
	void stop();
	void setState(SystemState::Ptr state);

	inline bool isRunning() const { return _running; }
	inline Hardware& getHardware() { return _hardware; }
	inline ElectionSystem& getElectionSystem() { return _electionSystem; }
	
	inline void setSocket(int sock) { _sock = sock; }
	inline int getSocket() { return _sock; }
	inline const Config& getConfig() { return _config; }
	inline void setConfig(Config& config) { _config = config; }
};


#endif /* SYSTEM_H_ */
