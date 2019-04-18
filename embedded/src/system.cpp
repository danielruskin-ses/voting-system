/* system.cpp */

#include "system.h"


using namespace std;

System::System() :
	_device(make_shared<Device>()),
	_electionSystem(make_shared<ElectionSystem>()),
	_state(make_shared<UninitializedState>())
{}

void System::start()
{
	_running = true; while (_running) {
		_state->update(shared_from_this());
	}
}

void System::stop()
{
	_running = false;
}

void System::notify(Key::Event::Ptr event)
{

}

