/* main.cpp */

#include "model/devicemodel.h"
#include "interface/interface.h"
#include "controller/devicecontroller.h"

int main(int argc, char **argv)
{
	DeviceModel device;
	Interface interface(device);
	DeviceController controller(device, interface);

	device.start();

	while (device.isRunning()) {
		controller.handleInput();
	}

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

