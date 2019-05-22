/* electionstate.cpp */

#include "electionstate.h"
#include "electionsystem.h"

#include <iostream>


using namespace std;

/* Verify user identity */
void AuthenticateState::init()
{
	// _interface->write("===== Vote Interface =====\n");
	// _interface->write("Enter last name: ");
	cout << "===== Vote Interface =====" << endl;
	cout << "Enter last name: " << endl;
}

void AuthenticateState::update()
{
	// _system.getDevice()->update();
	// _system.getElectionSystem()->update();
	// _interface->update(system);
	
	//string user = _system.getDevice()->scanString();
	_system.setState(make_shared<SelectionState>(_system));
}

void AuthenticateState::exit()
{
	cout << "Authenticated" << endl;
}


/* User inputs choices for election */
void SelectionState::init()
{
	// _interface->write("Enter candidate choice: ");
	cout << "Enter candidate choice: " << endl;
}

void SelectionState::update()
{
	// string choice = _system.getDevice()->scanString();
	// _interface->write("Choice: " + choice + "\n");
	
	_system.setState(make_shared<CastState>(_system));
}


/* Ballot is constructed, encrypted, and sent */
void CastState::init()
{
	cout << "Casting ballot..." << endl;
}

void CastState::update()
{
	cout << "Ballot cast" << endl;
	// _interface->write("Shutdown (y/n)? ");
	//string decision = _system.getDevice()->scanString();
	/*if (decision.find("y") != -1) {
		//_system.setState(make_shared<ShutdownState>(_system));
	} else {
		_system.setState(make_shared<AuthenticateState>(_system));
	}*/
	
	_system.setState(make_shared<AuthenticateState>(_system));
}
