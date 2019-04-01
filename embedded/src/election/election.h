/*
 * election.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_ELECTION_ELECTION_H_
#define SRC_ELECTION_ELECTION_H_

#include <list>

class VotingItem;

class Election
{
private:
	long long _openTime;	// time at which election begins
	long long _closeTime;	// time at which election ends
	std::list<VotingItem> _items;	// list of different voting items (e.g. President, VP, Representative)
public:
	Election(long long openTime, long long closeTime, const std::list<VotingItem>& items);
	virtual ~Election();

	inline long long getOpenTime() const { return _openTime; }
	inline long long getCloseTime() const { return _closeTime; }
	inline const std::list<VotingItem>& getItems() const { return _items; }
};

#endif /* SRC_ELECTION_ELECTION_H_ */
