/*
 * connection.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef NETWORK_CONNECTION_H_
#define NETWORK_CONNECTION_H_

#include "networkinterface.h"

#include <memory>

class Election;
class EncryptedBallot;

class Connection
{
private:
	NetworkInterface& _interface;
public:
	Connection(NetworkInterface& _interface);
	virtual ~Connection();

	std::unique_ptr<Election> getElection();
	bool sendBallot(const EncryptedBallot& ballot);
};

#endif /* NETWORK_CONNECTION_H_ */
