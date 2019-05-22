/* interface.cpp */

#include "interface.h"

#include "../system.h"

#include <iostream>


using namespace std;

void SetupInterface::update(System::Ptr system)
{
	if (!_displayed) {
		cout << "===== Setup interface =====" << endl;
		cout << "Enter server IP: ";
		_displayed = true;
	}
}




void VoteInterface::update(System::Ptr system)
{
	if (!_displayed) {
		cout << "===== Vote interface =====" << endl;
		cout << "Enter user: ";
		_displayed = true;
	}
}

