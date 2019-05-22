/* electionstate.h */

#ifndef ELECTION_ELECTIONSTATE_H_
#define ELECTION_ELECTIONSTATE_H_


#include <memory>


class ElectionSystem;

class ElectionState
{
protected:
	ElectionSystem& _system;
public:
	typedef std::shared_ptr<ElectionState> Ptr;

	ElectionState(ElectionSystem& system) :
	_system(system) {}
	virtual ~ElectionState() {}

	virtual void init() {}
	virtual void update() {}
	virtual void exit() {}
};


class AuthenticateState : public ElectionState
{
public:
	AuthenticateState(ElectionSystem& system) :
	ElectionState(system) {}
	virtual ~AuthenticateState() {}
	
	virtual void init();
	virtual void update();
	virtual void exit();
};


class SelectionState : public ElectionState
{
public:
	SelectionState(ElectionSystem& system) :
	ElectionState(system) {}
	virtual ~SelectionState() {}
	
	virtual void init();
	virtual void update();
};


class CastState : public ElectionState
{
public:
	CastState(ElectionSystem& system) :
	ElectionState(system) {}
	virtual ~CastState() {}
	
	virtual void init();
	virtual void update();
};


#endif /* ELECTION_ELECTIONSTATE_H_ */

