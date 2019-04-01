/*
 * connection.cpp
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#include "connection.h"

#include "../election/election.h"
#include "../election/encryptedballot.h"

Connection::Connection(NetworkInterface& interface) :
	_interface(interface)
{
	// TODO Auto-generated constructor stub

}

Connection::~Connection()
{
	// TODO Auto-generated destructor stub
}

std::unique_ptr<Election> Connection::getElection()
{
	return nullptr; // TODO
}

bool Connection::sendBallot(const EncryptedBallot& ballot)
{
	return false;	// TODO
}
