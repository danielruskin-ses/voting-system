/*
 * encryptedballot.cpp
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#include "encryptedballot.h"

#include "ballot.h"

EncryptedBallot::EncryptedBallot(const Ballot& ballot) :
	_data(encryptBallot(ballot))
{

}

EncryptedBallot::~EncryptedBallot()
{
	// TODO Auto-generated destructor stub
}

char *EncryptedBallot::encryptBallot(const Ballot& ballot) const
{
	return nullptr;	// TODO
}
