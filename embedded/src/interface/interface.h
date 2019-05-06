/* interface.h */

#ifndef INTERFACE_H_
#define INTERFACE_H_


#include "../hardware/keypad.h"

#include <memory>


class System;

class Interface
{
public:
	typedef std::shared_ptr<Interface> Ptr;

	Interface() {}
	virtual ~Interface() {}

	virtual void update(std::shared_ptr<System> system) {};
	virtual void handleKey(std::shared_ptr<System> system, Keypad::Event::Ptr event) {};
};


class SetupInterface : public Interface
{
private:
	bool _displayed;
public:
	SetupInterface() {}
	virtual ~SetupInterface() {}

	virtual void update(std::shared_ptr<System> system);
	virtual void handleKey(std::shared_ptr<System> system, Keypad::Event::Ptr event);
};


class VoteInterface : public Interface
{
private:
	bool _displayed;
public:
	VoteInterface() {}
	virtual ~VoteInterface() {}

	virtual void update(std::shared_ptr<System> system);
};


#endif /* INTERFACE_H_ */
