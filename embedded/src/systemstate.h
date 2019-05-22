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
	System& _system;
	Interface::Ptr _interface;
public:
	typedef std::shared_ptr<SystemState> Ptr;

	SystemState(System& system, Interface::Ptr interface=nullptr) :
		_system(system),
		_interface(interface)
	{}
	virtual ~SystemState()
	{}

	virtual void init() {}
	virtual void update() {}
	virtual void exit() {}
	virtual void handleKey(Keypad::Event::Ptr event) {}
};


class UninitializedState : public SystemState
{
public:
	UninitializedState(System& system) :
	SystemState(system) {}
	virtual ~UninitializedState() {}

	virtual void init();
	virtual void exit();
};


class SetupState : public SystemState
{
public:
	SetupState(System& system) :
	SystemState(system, std::make_shared<SetupInterface>()) {}
	virtual ~SetupState() {}

	virtual void init();
	virtual void update();
};


class ConnectingState : public SystemState
{
public:
	ConnectingState(System& system) : SystemState(system) {}
	virtual ~ConnectingState() {}

	virtual void init();
	virtual void update();
	virtual void exit();
};


class DownloadState : public SystemState
{
public:
	DownloadState(System& system) : SystemState(system) {}
	virtual ~DownloadState() {}
	
	virtual void init();
	virtual void update();
};


class RunState : public SystemState
{
public:
	RunState(System& system) : SystemState(system) {}
	virtual ~RunState() {}
	
	virtual void init();
	virtual void update();
	virtual void exit();
};


class ShutdownState : public SystemState
{
public:
	ShutdownState(System& system) :
	SystemState(system) {}
	virtual ~ShutdownState() {}

	virtual void init();
};


#endif /* SYSTEMSTATE_H_ */
