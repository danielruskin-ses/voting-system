#include "TreeGen.shared.h"
#include "Crypto.shared.h"

void treeGen(const std::vector<SignedRecordedBallot>& signedRecordedBallotsSorted, Tree* outputTree) {
        if(signedRecordedBallotsSorted.size() > 0) {
                treeGenImpl(signedRecordedBallotsSorted, outputTree, 0, signedRecordedBallotsSorted.size() - 1);        
        }
}

void treeGenImpl(const std::vector<SignedRecordedBallot>& signedRecordedBallotsSorted, Tree* outputTree, int start, int end) {
        // Base case
        if(start > end) {
                return;
        }

        // Set root
        int middleElem = ((end - start) / 2) + start;
        outputTree->mutable_root()->mutable_treenode()->mutable_signedrecordedballot()->CopyFrom(signedRecordedBallotsSorted.at(middleElem));

        // Set left child, right child
        treeGenImpl(signedRecordedBallotsSorted, outputTree->mutable_left(), start, middleElem - 1);
        treeGenImpl(signedRecordedBallotsSorted, outputTree->mutable_right(), middleElem + 1, end);

        // Set children hashes of root
        if(outputTree->left().has_root()) {
                Hash* hash = outputTree->mutable_root()->mutable_treenode()->add_childrenhashes();
                hash->set_hash(outputTree->left().root().hash().hash());
        }
        if(outputTree->right().has_root()) {
                Hash* hash = outputTree->mutable_root()->mutable_treenode()->add_childrenhashes();
                hash->set_hash(outputTree->right().root().hash().hash());
        }

        // Set hash of root
        HashMessage(
                outputTree->root().treenode(),
                outputTree->mutable_root()->mutable_hash()
        );
}

void getPartialTree(const Tree& tree, int targetVoterDeviceId, Tree* outputTree) {
        if(!tree.has_root()) {
                throw std::runtime_error("Target voter device does not have a ballot in the tree!");
        }

        // Copy BMT root => PBMT root
        outputTree->mutable_root()->CopyFrom(tree.root());

        // Finish, recurse to left, or recurse to right, depending on whether target =, <, > current node voter device ID.
        int currentNodeVoterDeviceId = tree.root().treenode().signedrecordedballot().recordedballot().signedproposedballot().proposedballot().voterdeviceid();
        if(targetVoterDeviceId == currentNodeVoterDeviceId) {
                return;
        } else if(targetVoterDeviceId > currentNodeVoterDeviceId) {
                getPartialTree(tree.right(), targetVoterDeviceId, outputTree->mutable_right());
        } else {
                getPartialTree(tree.left(), targetVoterDeviceId, outputTree->mutable_left());
        }
}
