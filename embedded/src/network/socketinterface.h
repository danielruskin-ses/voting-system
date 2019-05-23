/*
 * socket.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef NETWORK_SOCKETINTERFACE_H_
#define NETWORK_SOCKETINTERFACE_H_

#include "networkinterface.h"

class SocketInterface: public NetworkInterface
{
private:
	int _socket;
public:
	SocketInterface();
	virtual ~SocketInterface();

	virtual std::unique_ptr<Connection> connect(const std::string& host, int port);
	virtual void disconnect();
	virtual int send(char *data, int len);
	virtual int receive(char *data, int len);
};

#endif /* NETWORK_SOCKETINTERFACE_H_ */
