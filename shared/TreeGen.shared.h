#pragma once

#include "vote_server.grpc.pb.h"

// Input: list of recorded ballots sorted by voter device ID
// Output: binary merkle tree with the requested ballots
void treeGen(const std::vector<SignedRecordedBallot>& signedRecordedBallotsSorted, Tree* outputTree);

// Input: list of sorted recorded ballots,
//        tree to place output in,
//        first recorded ballot to consider (inclusive)
//        second recorded ballot to consider (inclusive)
void treeGenImpl(const std::vector<SignedRecordedBallot>& signedRecordedBallotsSorted, Tree* outputTree, int start, int end);

HashedTreeNode findNodeForVoterDeviceId(const Tree& tree, int targetVoterDeviceId);
void getPartialTree(const Tree& tree, int targetVoterDeviceId, Tree* outputTree);

bool verifyTreeStructure(const Tree& tree);
bool verifyTreeStructureImpl(const Tree& tree, int minId, int maxId);

bool verifyPartialTreeStructure(const Tree& tree);
bool verifyPartialTreeStructureImpl(const Tree& tree, int minId, int maxId);

// Do two trees contain the same ballots?
bool checkTreeBallots(const Tree& treeA, const Tree& treeB);
