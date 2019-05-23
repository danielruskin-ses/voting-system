/* electionstate.cpp */

#include "electionstate.h"
#include "electionsystem.h"

#include <iostream>


using namespace std;

/* Verify user identity */
void AuthenticateState::init()
{
	cout << "===== Vote Interface =====" << endl;
	cout << "Enter last name: " << endl;
}

void AuthenticateState::update()
{
	_system.setState(make_shared<SelectionState>(_system));
}

void AuthenticateState::exit()
{
	cout << "Authenticated" << endl;
}


/* User inputs choices for election */
void SelectionState::init()
{
	cout << "Enter candidate choice: " << endl;
}

void SelectionState::update()
{
	_system.setState(make_shared<CastState>(_system));
}


/* Ballot is constructed, encrypted, and sent */
void CastState::init()
{
	cout << "Casting ballot..." << endl;
	
	cout << "Ballot cast" << endl;
}

void CastState::update()
{
	_system.setState(make_shared<AuthenticateState>(_system));
}
