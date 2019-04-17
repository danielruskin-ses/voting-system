/*
 * keypad.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_HARDWARE_KEYPAD_H_
#define SRC_HARDWARE_KEYPAD_H_


#include <list>


class Keypad
{
public:
	class Event
	{

	};

	class Listener
	{
	public:
		typedef std::shared_ptr<Listener> Ptr;

		Listener() {}
		~Listener() {}

		virtual void notify(Keypad::Event event) = 0;
	};
private:
	std::list<std::shared_ptr<Keypad::Listener>> _listeners;
public:
	Keypad() {}
	virtual ~Keypad() {}

	void poll();
	void registerListener(Keypad::Listener::Ptr listener);
};

#endif /* SRC_HARDWARE_KEYPAD_H_ */
