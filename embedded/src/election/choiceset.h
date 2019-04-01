/*
 * choiceset.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 *
 *  A set of mutually exclusive choices
 */

#ifndef SRC_ELECTION_CHOICESET_H_
#define SRC_ELECTION_CHOICESET_H_

#include "choice.h"

#include <unordered_set>

class ChoiceSet
{
private:
	std::unordered_set<Choice> _choices;
public:
	ChoiceSet();
	virtual ~ChoiceSet();

	bool addChoice(const Choice& choice);
	bool removeChoice(const Choice& choice);
	bool containsChoice(const Choice& choice);

	inline bool operator==(const ChoiceSet& other) const { return _choices == other._choices; }
};

#endif /* SRC_ELECTION_CHOICESET_H_ */
