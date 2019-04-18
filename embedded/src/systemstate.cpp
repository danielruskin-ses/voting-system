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
}


void SetupState::update(System::Ptr system)
{
	cout << "Execute setup update" << endl;

	system->getDevice()->update();
}

void SetupState::handleKey(System::Ptr system, Key::Event::Ptr event)
{
	system->setState(make_shared<RunState>());
}


void RunState::update(System::Ptr system)
{
	cout << "Execute run update" << endl;

	system->getDevice()->update();
	system->getElectionSystem()->update();
}

void RunState::handleKey(System::Ptr system, Key::Event::Ptr event)
{
	system->setState(make_shared<ShutdownState>());
}


void ShutdownState::update(System::Ptr system)
{
	cout << "System shutting down" << endl;

	system->getDevice()->shutdown();
	system->setState(make_shared<UninitializedState>());
	system->stop();
}

