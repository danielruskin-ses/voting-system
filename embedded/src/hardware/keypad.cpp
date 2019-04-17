/*
 * keypad.cpp
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#include "keypad.h"

#include <algorithm>


using namespace std;

void Keypad::poll()
{
	list<Keypad::Event> events;
	// iterate through keys
	//     add events
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

