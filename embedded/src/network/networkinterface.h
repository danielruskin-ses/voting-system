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

class NetworkInterface
{
public:
	NetworkInterface() {}
	virtual ~NetworkInterface() {}

	virtual std::unique_ptr<Connection> connect() = 0;
	virtual void disconnect(std::unique_ptr<Connection> connection) = 0;
};

#endif /* NETWORK_NETWORKINTERFACE_H_ */
