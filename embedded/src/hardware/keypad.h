/*
 * keypad.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_HARDWARE_KEYPAD_H_
#define SRC_HARDWARE_KEYPAD_H_


#include <list>


class Key
{
public:
	class Event
	{
	public:
		typedef std::shared_ptr<Key::Event> Ptr;

		Event() {}
		virtual ~Event() {}
	};
private:
	bool _changed;
public:
	Key() {}
	~Key() {}

	void poll();

	bool hasEvent() const;
	Key::Event::Ptr getEvent();
};


class Keypad
{
public:
	class Listener
	{
	public:
		typedef std::shared_ptr<Listener> Ptr;

		Listener() {}
		~Listener() {}

		virtual void notify(Key::Event::Ptr event) = 0;
	};
private:
	std::list<Key> _keys;	
	std::list<std::shared_ptr<Keypad::Listener>> _listeners;
public:
	Keypad() {}
	virtual ~Keypad() {}

	void poll();
	void registerListener(Keypad::Listener::Ptr listener);
};


#endif /* SRC_HARDWARE_KEYPAD_H_ */
