/*
 * connection.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef NETWORK_CONNECTION_H_
#define NETWORK_CONNECTION_H_

#include <memory>

class NetworkInterface;

class Connection
{
private:
	NetworkInterface& _interface;
public:
	Connection(NetworkInterface& interface) : _interface(interface) {}
	~Connection() { _interface.disconnect() }

	bool send(Command command);
	Response receive();
};

#endif /* NETWORK_CONNECTION_H_ */
