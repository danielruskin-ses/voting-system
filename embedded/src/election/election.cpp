/*
 * election.cpp
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#include "election.h"

#include "votingitem.h"

Election::Election(long long openTime, long long closeTime, const std::list<VotingItem>& items) :
	_openTime(openTime),
	_closeTime(closeTime),
	_items(items)
{
	// TODO Auto-generated constructor stub

}

Election::~Election()
{
	// TODO Auto-generated destructor stub
}
