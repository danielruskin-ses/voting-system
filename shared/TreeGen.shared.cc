#include "TreeGen.shared.h"
#include "Crypto.shared.h"

Tree treeGen(std::vector<RecordedBallot> recordedBallotsSorted) {
        Tree outputTree;
        
        if(recordedBallotsSorted.size() > 0) {
                treeGenImpl(recordedBallotsSorted, &outputTree, 0, recordedBallotsSorted.size() - 1);        
        }

        return outputTree;
}

void treeGenImpl(std::vector<RecordedBallot> recordedBallotsSorted, Tree* outputTree, int start, int end) {
        // Base case
        if(start < end) {
                return;
        }

        // Set root
        int middleElem = ((end - start) / 2) + start;
        outputTree->mutable_root()->mutable_recordedballot()->CopyFrom(recordedBallotsSorted[middleElem]);

        // Set left child, right child
        treeGenImpl(recordedBallotsSorted, outputTree->mutable_left(), start, middleElem - 1);
        treeGenImpl(recordedBallotsSorted, outputTree->mutable_right(), middleElem + 1, end);

        // Set children hashes of root
        if(outputTree->left().has_root()) {
                Hash* hash = outputTree->mutable_root()->add_childrenhashes();
                hash->set_hash(outputTree->left().root().hash().hash());
        }
        if(outputTree->right().has_root()) {
                Hash* hash = outputTree->mutable_root()->add_childrenhashes();
                hash->set_hash(outputTree->right().root().hash().hash());
        }

        // Set hash of root
        std::string serializedRoot;
        outputTree->root().SerializeToString(&serializedRoot);
        outputTree->mutable_root()->mutable_hash()->set_hash(
                HashMessage(serializedRoot)
        );
}
