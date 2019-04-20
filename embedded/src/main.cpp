/* main.cpp */

#include "system.h"

#include <memory>

#include <cstdlib>
#include <ctime>


using namespace std;

int main(int argc, char **argv)
{
	srand(time(0));
	System::Ptr system = make_shared<System>();
	system->start();
		/*
		 * hardware.setup();

		EthernetAdapter ethernetAdapter = hardware.getEthernetAdapter();
		WiFiAdapter wifiAdapter = hardware.getWiFiAdapter();

		ElectionServer electionServer;
		Election election = electionServer.getElection();
				 * initHardware();
				 *
				 * setup();
				 *
				 * Election election = electionServer.getElection();
				 *
				 * Voter voter("Davis", "Jonathan");
				 * Ballot ballot(voter);
				 *
				 * for (VotingItem item : election.getVotingItems()) {
				 * 	   Choice choice = VOTER_SELECT_CHOICE();
				 * 	   ballot.setChoice(item, choice);
				 * }
				 *
				 * EncryptedBallot encrypted(ballot);
				 * electionServer.sendBallot(encrypted);
				 */

	return 0;
};

