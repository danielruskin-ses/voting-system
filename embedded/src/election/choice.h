/*
 * choice.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_ELECTION_CHOICE_H_
#define SRC_ELECTION_CHOICE_H_

#include <functional>
#include <string>

class Choice
{
private:
	std::string _choice;
public:
	Choice(const std::string& choice);
	virtual ~Choice();

	inline std::string getChoice() const { return _choice; }
	inline bool operator==(const Choice& other) const { return _choice == other._choice; }
};

namespace std
{
	template <>
	struct hash<Choice>
	{
		size_t operator()(const Choice& choice) const
		{
			return hash<string>()(choice.getChoice());
		}
	};
}

#endif /* SRC_ELECTION_CHOICE_H_ */
