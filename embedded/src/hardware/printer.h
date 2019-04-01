/*
 * printer.h
 *
 *  Created on: Mar 22, 2019
 *      Author: jonathandavis
 */

#ifndef SRC_HARDWARE_PRINTER_H_
#define SRC_HARDWARE_PRINTER_H_

class Receipt;

class Printer
{
public:
	Printer();
	virtual ~Printer();

	void printReceipt(const Receipt& receipt);
};

#endif /* SRC_HARDWARE_PRINTER_H_ */
