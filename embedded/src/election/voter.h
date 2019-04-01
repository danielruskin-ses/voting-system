/*
 * voter.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_ELECTION_VOTER_H_
#define SRC_ELECTION_VOTER_H_

#include <string>

class Voter
{
private:
	std::string _lastName;
	std::string _firstName;
public:
	Voter(const std::string& lastName, const std::string& firstName);
	virtual ~Voter();

	inline std::string getLastName() const { return _lastName; }
	inline std::string getFirstName() const { return _firstName; }
};

#endif /* SRC_ELECTION_VOTER_H_ */
