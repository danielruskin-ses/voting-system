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
	Connection(NetworkInterface& _interface);
	virtual ~Connection();

	virtual int send(char *data);
	virtual int receive(char *data);
};

#endif /* NETWORK_CONNECTION_H_ */
