/*
 * keypad.cpp
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#include "keypad.h"

#include <algorithm>


using namespace std;

bool Key::poll(const GPIO& gpio)
{
	return gpio.getInput(_pin);
}

	
bool operator<(const Key& k1, const Key& k2)
{
	return k1._pin < k2._pin;
}


void Keypad::poll(const GPIO& gpio)
{
	list<Keypad::Event::Ptr> events;

	for (auto it = _keys.begin(); it != _keys.end(); ++it) {
		Key key = it->first;
		bool state = it->second;
		bool newState = key.poll(gpio);

		if (!state && newState) {
			// Key pressed
			events.push_back(make_shared<Keypad::Event>());
		} else if (state && !newState) {
			// Key released
			events.push_back(make_shared<Keypad::Event>());
		}
		_keys[key] = newState;
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

