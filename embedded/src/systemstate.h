/* systemstate.h */

#ifndef SYSTEMSTATE_H_
#define SYSTEMSTATE_H_


#include "hardware/keypad.h"
#include "interface/interface.h"

#include <memory>


class System;

class SystemState
{
protected:
	Interface::Ptr _interface;
public:
	typedef std::shared_ptr<SystemState> Ptr;

	SystemState() {}
	virtual ~SystemState() {}

	virtual void update(std::shared_ptr<System> system) {}
	virtual void handleKey(std::shared_ptr<System> system, Keypad::Event::Ptr event) {}
};


class UninitializedState : public SystemState
{
public:
	UninitializedState() {}
	virtual ~UninitializedState() {}

	virtual void update(std::shared_ptr<System> system);
};


class SetupState : public SystemState
{
public:
	SetupState() { _interface = std::make_shared<SetupInterface>(); }
	virtual ~SetupState() {}

	virtual void update(std::shared_ptr<System> system);
	virtual void handleKey(std::shared_ptr<System> system, Keypad::Event::Ptr event);
};


class RunState : public SystemState
{
public:
	RunState() { _interface = std::make_shared<VoteInterface>(); }
	virtual ~RunState() {}

	virtual void update(std::shared_ptr<System> system);
	virtual void handleKey(std::shared_ptr<System> system, Keypad::Event::Ptr event);
};


class ShutdownState : public SystemState
{
public:
	ShutdownState() {}
	virtual ~ShutdownState() {}

	virtual void update(std::shared_ptr<System> system);
};


#endif /* SYSTEMSTATE_H_ */
