/* system.cpp */

#include "system.h"


using namespace std;

void System::start()
{
	_running = true;
	setState(make_shared<UninitializedState>(*this));
	while (_running) {
		_state->update();
	}
}

void System::stop()
{
	_running = false;
}

void System::setState(SystemState::Ptr state)
{
	if (_state != nullptr) {
		_state->exit();
	}
	_state = state;
	if (_running) {
		_state->init();
	}
}

