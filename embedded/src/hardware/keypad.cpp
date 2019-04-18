/*
 * keypad.cpp
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#include "keypad.h"

#include <algorithm>


using namespace std;

void Key::poll()
{

}

bool Key::hasEvent() const
{
	return _changed;
}

Key::Event::Ptr Key::getEvent()
{
	_changed = false;
	return nullptr;
}


void Keypad::poll()
{
	list<Key::Event::Ptr> events;

	for (auto it = _keys.begin(); it != _keys.end(); ++it) {
		Key key = *it;
		if (key.hasEvent()) {
			events.push_back(key.getEvent());
		}
	}

	for (auto it = _listeners.begin(); it != _listeners.end(); ++it) {
		for (auto eventIt = events.begin(); eventIt != events.end(); ++eventIt) {
			auto listener = *it;
			auto event = *eventIt;
			listener->notify(event);
		}
	}
};

void Keypad::registerListener(Keypad::Listener::Ptr listener)
{
	if (find(_listeners.begin(), _listeners.end(), listener) != _listeners.end()) {
		_listeners.push_back(listener);
	}
}

