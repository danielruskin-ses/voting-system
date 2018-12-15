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
                hash->CopyFrom(outputTree->left().root().hash());
        }
        if(outputTree->right().has_root()) {
                Hash* hash = outputTree->mutable_root()->mutable_treenode()->add_childrenhashes();
                hash->CopyFrom(outputTree->right().root().hash());
        }

        // Set hash of root
        HashMessage(
                outputTree->root().treenode(),
                outputTree->mutable_root()->mutable_hash()
        );
}

HashedTreeNode findNodeForVoterDeviceId(const Tree& tree, int targetVoterDeviceId) {
        if(!tree.has_root()) {
                throw std::runtime_error("Target voter device does not have a ballot in the tree!");
        }

        int currentNodeVoterDeviceId = tree.root().treenode().signedrecordedballot().recordedballot().signedproposedballot().proposedballot().voterdeviceid();
        if(targetVoterDeviceId == currentNodeVoterDeviceId) {
                return tree.root();
        } else if(targetVoterDeviceId > currentNodeVoterDeviceId) {
                return findNodeForVoterDeviceId(tree.right(), targetVoterDeviceId);
        } else {
                return findNodeForVoterDeviceId(tree.left(), targetVoterDeviceId);
        }
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

bool verifyTreeStructure(const Tree& tree) {
        verifyTreeStructureImpl(tree, -1, -1);
}

bool verifyTreeStructureImpl(const Tree& tree, int minId, int maxId) {
        // If no root, verify end of tree.
        if(!tree.has_root()) {
                return !(tree.has_left() || tree.has_right());
        }

        // If root:
        // 1. Verify root has correct # of child hashes.
        int correctNum = 0;
        if(tree.left().has_root() && tree.right().has_root()) {
                correctNum = 2;
        } else if(tree.left().has_root() || tree.right().has_root()) {
                correctNum = 1;
        }
        if(tree.root().treenode().childrenhashes_size() != correctNum) {
                return false;
        }

        // 2. Verify root has correct child hashes.
        if(tree.left().has_root()) {
                bool leftHashCorrect = google::protobuf::util::MessageDifferencer::Equals(
                        tree.root().treenode().childrenhashes(0),
                        tree.left().root().hash()
                );
                if(!leftHashCorrect) {
                        return false;
                }
        }
        if(tree.right().has_root()) {
                bool rightHashCorrect = google::protobuf::util::MessageDifferencer::Equals(
                        tree.root().treenode().childrenhashes(tree.root().treenode().childrenhashes_size() - 1),
                        tree.right().root().hash()
                );
                if(!rightHashCorrect) {
                        return false;
                }
        }

        // 3. Verify root has correct hash
        bool validRootHash = VerifyHash(
                tree.root().treenode(),
                tree.root().hash()
        );
        if(!validRootHash) {
                return false;
        }
                
        // 4. Verify root has voter device ID between min/max
        int currNodeId = tree.root().treenode().signedrecordedballot().recordedballot().signedproposedballot().proposedballot().voterdeviceid();
        if(minId != -1 && currNodeId < minId) {
                return false;
        }
        if(maxId != -1 && currNodeId > maxId) {
                return false;
        }

        // 5. Recurse to left, right
        return 
                verifyTreeStructureImpl(
                        tree.left(), 
                        minId,
                        currNodeId
                ) && verifyTreeStructureImpl(
                        tree.right(), 
                        currNodeId,
                        maxId
                );
}

bool checkTreeBallots(const Tree& treeA, const Tree& treeB) {
        // Tree roots must be equal
        bool sameRoot = google::protobuf::util::MessageDifferencer::Equals(treeA.root().treenode().signedrecordedballot(), treeB.root().treenode().signedrecordedballot());
        if(!sameRoot) {
                return false;
        }

        // Tree left, right must be equal
        if(treeA.has_left() || treeB.has_left()) {
                bool res = checkTreeBallots(treeA.left(), treeB.left());
                if(!res) {
                        return false;
                }
        }
        if(treeA.has_right() || treeB.has_right()) {
                bool res = checkTreeBallots(treeA.right(), treeB.right());
                if(!res) {
                        return false;
                }
        }

        return true;
}

bool verifyPartialTreeStructure(const Tree& tree) {
        verifyPartialTreeStructureImpl(tree, -1, -1);
}

bool verifyPartialTreeStructureImpl(const Tree& tree, int minId, int maxId) {
        // If no root, verify end of tree.
        if(!tree.has_root()) {
                return !(tree.has_left() || tree.has_right());
        }

        // If root:
        // 1. Verify root has <2 children
        if(tree.left().has_root() && tree.right().has_root()) {
                return false;
        }

        // 2. Verify root has correct child hashes.
        if(tree.left().has_root()) {
                bool leftHashCorrect = google::protobuf::util::MessageDifferencer::Equals(
                        tree.root().treenode().childrenhashes(0),
                        tree.left().root().hash()
                );
                if(!leftHashCorrect) {
                        return false;
                }
        }
        if(tree.right().has_root()) {
                bool rightHashCorrect = google::protobuf::util::MessageDifferencer::Equals(
                        tree.root().treenode().childrenhashes(tree.root().treenode().childrenhashes_size() - 1),
                        tree.right().root().hash()
                );
                if(!rightHashCorrect) {
                        return false;
                }
        }

        // 3. Verify root has correct hash
        bool validRootHash = VerifyHash(
                tree.root().treenode(),
                tree.root().hash()
        );
        if(!validRootHash) {
                return false;
        }
                
        // 4. Verify root has voter device ID between min/max
        int currNodeId = tree.root().treenode().signedrecordedballot().recordedballot().signedproposedballot().proposedballot().voterdeviceid();
        if(minId != -1 && currNodeId < minId) {
                return false;
        }
        if(maxId != -1 && currNodeId > maxId) {
                return false;
        }

        // 5. Recurse to left, right
        return 
                verifyTreeStructureImpl(
                        tree.left(), 
                        minId,
                        currNodeId
                ) && verifyTreeStructureImpl(
                        tree.right(), 
                        currNodeId,
                        maxId
                );
}
