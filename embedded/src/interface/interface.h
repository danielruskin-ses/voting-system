/* interface.h */

#ifndef INTERFACE_H_
#define INTERFACE_H_


#include "../hardware/keypad.h"

#include <iostream>
#include <memory>
#include <string>


class System;

class Interface
{
public:
	typedef std::shared_ptr<Interface> Ptr;

	Interface() {}
	virtual ~Interface() {}

	void write(const std::string& s) { std::cout << s; }
	virtual void update(std::shared_ptr<System> system) {};
};


class SetupInterface : public Interface
{
private:
	bool _displayed;
public:
	SetupInterface() {}
	virtual ~SetupInterface() {}

	virtual void update(std::shared_ptr<System> system);
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
