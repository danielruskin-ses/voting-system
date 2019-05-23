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


using namespace std;

const int timeout = 5;

SocketInterface::SocketInterface() :
	_socket(socket(AF_INET, SOCK_STREAM, 0))
{

}

SocketInterface::~SocketInterface()
{
	close(_socket);
}

std::unique_ptr<Connection> SocketInterface::connect(const string& host, int port)
{
	sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	int result = inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);
	if (result <= 0) {
		cout << "Failed to resolve host" << endl;
		return nullptr;
	}
	
	result = connect(_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	
	if (result < 0) {
		cout << "Failed to connect to host" << endl;
		return nullptr;
	}
	
	return move(make_unique<Connection>(*this));
}

void SocketInterface::disconnect()
{
	close(_socket);
}

int SocketInterface::send(char *data, int len)
{
	return socketSend(_socket, data, len, timeout);
}

int SocketInterface::recv(char *data, int len)
{
	return socketRecv(_socket, data, len, timeout);
}
