/*
 * keypad.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_HARDWARE_KEYPAD_H_
#define SRC_HARDWARE_KEYPAD_H_


#include "gpio.h"

#include <list>
#include <map>


class Key
{
private:
	int _pin;
public:
	Key(int pin) : _pin(pin) {}
	~Key() {}

	bool poll(const GPIO& gpio);

	friend bool operator<(const Key& k1, const Key& k2);
};


class Keypad
{
public:
	class Event
	{
	public:
		typedef std::shared_ptr<Keypad::Event> Ptr;

		Event() {}
		virtual ~Event() {}
	};
	
	class Listener
	{
	public:
		typedef std::shared_ptr<Listener> Ptr;

		Listener() {}
		~Listener() {}

		virtual void notify(Keypad::Event::Ptr event) = 0;
	};
private:
	std::map<Key, bool> _keys;	
	std::list<std::shared_ptr<Keypad::Listener>> _listeners;
public:
	Keypad() {}
	virtual ~Keypad() {}

	void poll(const GPIO& gpio);
	void registerListener(Keypad::Listener::Ptr listener);
};


#endif /* SRC_HARDWARE_KEYPAD_H_ */
