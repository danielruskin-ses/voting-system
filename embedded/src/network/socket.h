/*
 * socket.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef NETWORK_SOCKET_H_
#define NETWORK_SOCKET_H_

#include "networkinterface.h"

class Socket: public NetworkInterface
{
private:
	int _socket;
public:
	Socket();
	virtual ~Socket();

	virtual int send(char *data);
	virtual int receive(char *data);
};

#endif /* NETWORK_SOCKET_H_ */
