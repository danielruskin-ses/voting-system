/*
 * socket.cpp
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#include "socketinterface.h"

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

SocketInterface::SocketInterface() :
	_socket(socket(AF_INET, SOCK_STREAM, 0))
{

}

SocketInterface::~SocketInterface()
{
	close(_socket);
}

std::unique_ptr<Connection> SocketInterface::connect()
{
	return nullptr;
}

void SocketInterface::disconnect(std::unique_ptr<Connection> connection)
{

}
