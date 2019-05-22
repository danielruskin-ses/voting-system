/* electionsystem.cpp */

#include "electionsystem.h"

void ElectionSystem::setState(ElectionState::Ptr state)
{
	if (_state != nullptr) {
		_state->exit();
	}
	_state = state;
	_state->init();
}
