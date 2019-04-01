/*
 * votingitem.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_ELECTION_VOTINGITEM_H_
#define SRC_ELECTION_VOTINGITEM_H_

#include "choiceset.h"

#include <string>

class VotingItem
{
private:
	std::string _itemName;
	ChoiceSet _choiceSet;
public:
	VotingItem();
	virtual ~VotingItem();

	const std::string& getItemName() const { return _itemName; }
	const ChoiceSet& getChoiceSet() const { return _choiceSet; }

	inline bool operator==(const VotingItem& other) const { return _itemName == other._itemName && _choiceSet == other._choiceSet; }
};

namespace std
{
	template <>
	struct hash<VotingItem>
	{
		size_t operator()(const VotingItem& item) const
		{
			return hash<string>()(item.getItemName());
		}
	};
}

#endif /* SRC_ELECTION_VOTINGITEM_H_ */
