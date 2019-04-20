/* systemstate.cpp */

#include "systemstate.h"

#include "system.h"

#include <iostream>


using namespace std;

void UninitializedState::update(System::Ptr system)
{
	cout << "Initializing system" << endl;

	system->getDevice()->init();
	system->getDevice()->registerKeypadListener(system);
	system->setState(make_shared<SetupState>());

	cout << "System initialized" << endl;
}


void SetupState::update(System::Ptr system)
{
	system->getDevice()->update();
	_interface->update(system);
}

void SetupState::handleKey(System::Ptr system, Keypad::Event::Ptr event)
{
	if (event->keycode() == 0 && event->getAction() == Keypad::Action::Release) {
		system->setState(make_shared<RunState>());
	} else {
		_interface->handleKey(system, event);
	}
}


void RunState::update(System::Ptr system)
{
	system->getDevice()->update();
	system->getElectionSystem()->update();
}

void RunState::handleKey(System::Ptr system, Keypad::Event::Ptr event)
{
	if (event->keycode() == 0 && event->getAction() == Keypad::Action::Release) {
		system->setState(make_shared<ShutdownState>());
	} else {
		_interface->handleKey(system, event);
	}
}


void ShutdownState::update(System::Ptr system)
{
	cout << "System shutting down" << endl;

	system->getDevice()->shutdown();
	system->setState(make_shared<UninitializedState>());
	system->stop();
}

