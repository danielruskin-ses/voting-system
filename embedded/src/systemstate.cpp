/* systemstate.cpp */

#include "systemstate.h"

#include "system.h"
#include "election/electionstate.h"

#include <iostream>
#include <string>


using namespace std;

/* Perform hardware initialization */
void UninitializedState::init()
{
	cout << "Initializing system" << endl;
	
	_system.getDevice().init();
	_system.setState(make_shared<SetupState>(_system));
}

void UninitializedState::exit()
{
	cout << "System initialized" << endl;
}


/* Election administrator configuration
 *
 * Includes settings such as server IP
 */
void SetupState::init()
{
	// _interface->write("===== Setup Interface =====\n");
	// _interface->write("Enter server IP: ");
	cout << "===== Setup Interface =====" << endl;
	cout << "Enter server IP: ";
}

void SetupState::update()
{
	// _system.getDevice()->update();
	// _interface->update(system);

	
	string addr;
	addr = _system.getDevice().scanString();
	// ElectionSystemConfiguration config;
	// config.address = addr;
	// _system.getElectionSystem()->applyConfiguration(config);

	_system.setState(make_shared<ConnectingState>(_system));
}


/* Device connects to server */
void ConnectingState::init()
{
	cout << "Connecting..." << endl;
}

void ConnectingState::update()
{
	// Attempt to reach server until timeout
	_system.setState(make_shared<DownloadState>(_system));
}

void ConnectingState::exit()
{
	cout << "Connected" << endl;
}


/* Device downloads election data from server */
void DownloadState::init()
{
	cout << "Downloading election data..." << endl;
}

void DownloadState::update()
{
	// Attempt to download election data until timeout
	_system.setState(make_shared<RunState>(_system));
}

void DownloadState::exit()
{
	cout << "Downloaded election data" << endl;
}


/* Device enters election mode */
void RunState::init()
{
	auto authState = make_shared<AuthenticateState>(_system.getElectionSystem());
	_system.getElectionSystem().setState(authState);
}

void RunState::update()
{
	// Run election until shutdown
	_system.getElectionSystem().update();
}

void RunState::exit()
{
	
}


void ShutdownState::init()
{
	cout << "System shutting down..." << endl;
	
	_system.getDevice().shutdown();
	_system.stop();
	
	cout << "System shut down" << endl;
}
