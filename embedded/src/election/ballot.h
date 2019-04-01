/*
 * ballot.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_ELECTION_BALLOT_H_
#define SRC_ELECTION_BALLOT_H_

#include "votingitem.h"
#include "voter.h"
#include "choice.h"

#include <unordered_map>

class Election;
class Receipt;

class Ballot
{
private:
	Voter _voter;
	std::unordered_map<VotingItem, Choice> _choices;
public:
	Ballot(const Voter& voter);
	virtual ~Ballot();

	void setChoice(const VotingItem& item, const Choice& choice);
	bool validate(const Election& election);
	Receipt getReceipt() const;
};

#endif /* SRC_ELECTION_BALLOT_H_ */
