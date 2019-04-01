/*
 * receipt.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_ELECTION_RECEIPT_H_
#define SRC_ELECTION_RECEIPT_H_

class Printer;

class Receipt
{
public:
	Receipt();
	virtual ~Receipt();

	void print(const Printer& printer);
};

#endif /* SRC_ELECTION_RECEIPT_H_ */
