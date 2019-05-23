/*
 * networkinterface.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef NETWORK_NETWORKINTERFACE_H_
#define NETWORK_NETWORKINTERFACE_H_

#include "connection.h"

#include <memory>
#include <string>

class NetworkInterface
{
public:
	NetworkInterface() {}
	virtual ~NetworkInterface() {}

	virtual std::unique_ptr<Connection> connect(const std::string& host, int port) = 0;
	virtual void disconnect() = 0;
	virtual int send(char *data, int len) = 0;
	virtual int receive(char *data, int len) = 0;
};

#endif /* NETWORK_NETWORKINTERFACE_H_ */
