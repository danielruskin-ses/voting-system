/* interface.cpp */

#include "interface.h"

#include "../system.h"

#include <iostream>


using namespace std;

void SetupInterface::update(System::Ptr system)
{
	if (!_displayed) {
		cout << "Setup interface" << endl;
		_displayed = true;
	}
}


void SetupInterface::handleKey(System::Ptr system, Keypad::Event::Ptr event)
{
	switch (event->getAction()) {
	case Keypad::Action::Press:
		cout << "Pressed ";
		break;
	case Keypad::Action::Release:
		cout << "Released ";
		break;
	}
	cout << event->keycode() << endl;
}

