/* electionsystem.h */

#ifndef ELECTION_ELECTIONSYSTEM_H_
#define ELECTION_ELECTIONSYSTEM_H_

#include "election.h"
#include "electionserver.h"
#include "electionstate.h"
#include "../hardware/keypad.h"


class ElectionSystem : public Keypad::Listener, public std::enable_shared_from_this<ElectionSystem>
{
private:
	ElectionServer _electionServer;
	Election _election;

	ElectionState::Ptr _state;
public:
	typedef std::shared_ptr<ElectionSystem> Ptr;

	ElectionSystem() : _state(std::make_shared<AuthenticateState>()) {}
	virtual ~ElectionSystem() {}

	inline void update() { _state->update(shared_from_this()); }
	inline void setState(ElectionState::Ptr state) { _state = state; }

	virtual void notify(Key::Event::Ptr event);
};

#endif /* ELECTION_ELECTIONSYSTEM_H_ */

