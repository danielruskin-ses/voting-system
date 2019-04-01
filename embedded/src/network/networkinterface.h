/*
 * networkinterface.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef NETWORK_NETWORKINTERFACE_H_
#define NETWORK_NETWORKINTERFACE_H_

class NetworkInterface
{
public:
	NetworkInterface();
	virtual ~NetworkInterface();

	virtual int send(char *data) = 0;
	virtual int receive(char *data) = 0;
};

#endif /* NETWORK_NETWORKINTERFACE_H_ */
