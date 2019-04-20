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
	int _key;
	int _pin;
public:
	Key(int key, int pin) : _key(key), _pin(pin) {}
	~Key() {}

	bool poll(const GPIO& gpio);

	inline int keycode() const { return _key; }

	friend bool operator<(const Key& k1, const Key& k2);
};


class Keypad
{
public:
	enum class Action;
	class Event;

	class Listener
	{
	public:
		typedef std::shared_ptr<Listener> Ptr;

		Listener() {}
		~Listener() {}

		virtual void notify(std::shared_ptr<Event> event) = 0;
	};
private:
	std::map<Key, bool> _keys;	
	std::list<std::shared_ptr<Keypad::Listener>> _listeners;
public:
	Keypad();
	virtual ~Keypad() {}

	void poll(const GPIO& gpio);
	void registerListener(Keypad::Listener::Ptr listener);
};


class Keypad::Event
{
private:
	int _key;
	Keypad::Action _action;
public:
	typedef std::shared_ptr<Keypad::Event> Ptr;

	Event(int key, Keypad::Action action) : _key(key), _action(action) {}
	virtual ~Event() {}

	inline int keycode() const { return _key; }
	inline Keypad::Action getAction() const { return _action; }
};


enum class Keypad::Action
{
	Press, Release
};


#endif /* SRC_HARDWARE_KEYPAD_H_ */
