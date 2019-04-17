/* main.cpp */

#include "system.h"


int main(int argc, char **argv)
{
	System system;
	system.run();
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

