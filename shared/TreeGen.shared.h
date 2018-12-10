#pragma once

#include "vote_server.grpc.pb.h"

// Input: list of recorded ballots sorted by voter device ID
// Output: binary merkle tree with the requested ballots
Tree treeGen(std::vector<RecordedBallot> recordedBallotsSorted);

// Input: list of sorted recorded ballots,
//        tree to place output in,
//        first recorded ballot to consider (inclusive)
//        second recorded ballot to consider (inclusive)
void treeGenImpl(std::vector<RecordedBallot> recordedBallotsSorted, Tree* outputTree, int start, int end);
