/* electionsystem.h */

#ifndef ELECTION_ELECTIONSYSTEM_H_
#define ELECTION_ELECTIONSYSTEM_H_

#include "election.h"
#include "electionserver.h"
#include "electionstate.h"
#include "../hardware/keypad.h"


class ElectionSystem
{
private:
	ElectionServer _electionServer;
	Election _election;

	ElectionState::Ptr _state;
public:
	typedef std::shared_ptr<ElectionSystem> Ptr;

	ElectionSystem() {}
	virtual ~ElectionSystem() {}
	
	void setState(ElectionState::Ptr state);

	inline void update() { _state->update(); }
};

#endif /* ELECTION_ELECTIONSYSTEM_H_ */

