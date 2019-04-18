/* systemstate.h */

#ifndef SYSTEMSTATE_H_
#define SYSTEMSTATE_H_


#include "hardware/keypad.h"

#include <memory>


class System;

class SystemState
{
public:
	typedef std::shared_ptr<SystemState> Ptr;
	SystemState() {}
	virtual ~SystemState() {}

	virtual void update(std::shared_ptr<System> system) {}
	virtual void handleKey(std::shared_ptr<System> system, Key::Event::Ptr event) {}
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
	SetupState() {}
	virtual ~SetupState() {}

	virtual void update(std::shared_ptr<System> system);
	virtual void handleKey(std::shared_ptr<System> system, Key::Event::Ptr event);
};


class RunState : public SystemState
{
public:
	RunState() {}
	virtual ~RunState() {}

	virtual void update(std::shared_ptr<System> system);
	virtual void handleKey(std::shared_ptr<System> system, Key::Event::Ptr event);
};


class ShutdownState : public SystemState
{
public:
	ShutdownState() {}
	virtual ~ShutdownState() {}

	virtual void update(std::shared_ptr<System> system);
};


#endif /* SYSTEMSTATE_H_ */
