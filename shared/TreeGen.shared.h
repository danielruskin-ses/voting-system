#pragma once

#include "vote_server.grpc.pb.h"

// Input: list of recorded ballots sorted by voter device ID
// Output: binary merkle tree with the requested ballots
void treeGen(const std::vector<RecordedBallot>& recordedBallotsSorted, Tree* outputTree);

// Input: list of sorted recorded ballots,
//        tree to place output in,
//        first recorded ballot to consider (inclusive)
//        second recorded ballot to consider (inclusive)
void treeGenImpl(const std::vector<RecordedBallot>& recordedBallotsSorted, Tree* outputTree, int start, int end);

void getPartialTree(const Tree& tree, int targetVoterDeviceId, Tree* outputTree);
