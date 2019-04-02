/*
 * encryptedballot.h
 *
 *  Created on: Mar 31, 2019
 *      Author: jonathandavis
 */

#ifndef ELECTION_ENCRYPTEDBALLOT_H_
#define ELECTION_ENCRYPTEDBALLOT_H_

class Ballot;

class EncryptedBallot
{
private:
	const char *_data;

	char *encryptBallot(const Ballot& ballot) const;
public:
	EncryptedBallot(const Ballot& ballot);
	virtual ~EncryptedBallot();

	inline const char *getData() const { return _data; }
};

#endif /* ELECTION_ENCRYPTEDBALLOT_H_ */
