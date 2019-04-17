/* electionstate.h */

#ifndef ELECTION_ELECTIONSTATE_H_
#define ELECTION_ELECTIONSTATE_H_


#include <memory>


class ElectionSystem;

class ElectionState
{
public:
	typedef std::shared_ptr<ElectionState> Ptr;

	ElectionState() {}
	virtual ~ElectionState() {}

	virtual void update(std::shared_ptr<ElectionSystem> system) {}
};


class SetupState : public ElectionState
{
public:
	SetupState() {}
	virtual ~SetupState() {}

	virtual void update(std::shared_ptr<ElectionSystem> system);
};


class ConnectState : public ElectionState
{

};


class AuthenticateState : public ElectionState
{

};


class VoteState : public ElectionState
{

};


#endif /* ELECTION_ELECTIONSTATE_H_ */

