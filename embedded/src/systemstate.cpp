/* systemstate.cpp */

#include "systemstate.h"

#include "system.h"
#include "election/electionstate.h"

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;

/* Perform hardware initialization */
void UninitializedState::init()
{
	cout << "Initializing system" << endl;
	
	_system.getHardware().init();
	_system.setSocket(::socket(AF_INET, SOCK_STREAM, 0));
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
	Config config;
	cout << "Enter server IP: ";
	cin >> config._host;
	cout << "Enter server port: ";
	cin >> config._port;
	_system.setConfig(config);
}

void SetupState::update()
{
	// _system.getHardware()->update();
	// _interface->update(system);

	
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
	sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(_system.getConfig()._port);
	int result = inet_pton(AF_INET, _system.getConfig()._host.c_str(), &serv_addr.sin_addr);
	if (result <= 0) {
		cout << "Failed to resolve host" << endl;
		return;
	}
	
	result = ::connect(_system.getSocket(), (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	
	if (result < 0) {
		cout << "Failed to connect to host" << endl;
		return;
	}
	
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
	bool result = sendCommand(sock, CommandType_GET_ELECTIONS, paginationEnc.second);
	if (!result) {
		cout << "Unable to send command" << endl;
		return;
	}
	
	std::pair<bool, Response> response = getResponse(sock);
	if (!response.first) {
		cout << "Failed to get response" << endl;
		return;
	}
	
	if (resp.second.type != ResponseType_ELECTIONS) {
		cout << "Invalid response" << endl;
		return;
	}
	
	Elections electionsParsed;
	pb_istream_t pbBuf = pb_istream_from_buffer(&(resp.second.data.bytes[0]), resp.second.data.size);
	result = pb_decode_delimited(&pbBuf, Elections_fields, &electionsParsed);
	if (!result) {
		cout << "Failed to parse elections" << endl;
		return;
	}
	
	// Deal with parsed elections
	// TODO
	
	_system.setState(make_shared<RunState>(_system));
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
	
	_system.getHardware().shutdown();
	_system.stop();
	
	cout << "System shut down" << endl;
}
