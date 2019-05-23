# Embedded Software for Voting Device
Contains software representation (System)
The System is comprised of only two main components: the Hardware and the ElectionSystem.
It is the System's job to coordinate between the Hardware and the ElectionSystem.
The System does this through SystemStates.

## Hardware
The Hardware is a representation of the physical components of the device.
Mainly, it is used as an abstraction to get information from device components as well as for utilizing modules such as an Ethernet adapter.

## ElectionSystem
The ElectionSystem is device's internal representation of the election in which it is participating.
It is responsible for authenticating users, creating ballots, and casting ballots.
