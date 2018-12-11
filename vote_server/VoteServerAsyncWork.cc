#include "VoteServerAsyncWork.h"
#include "Crypto.shared.h"
#include "TreeGen.shared.h"

VoteServerAsyncWork::VoteServerAsyncWork(const Logger& logger, VoteServerDatabase& database) : _database(database), AsyncWork(logger) {
}

void VoteServerAsyncWork::loopInner() {
        ElectionMetadata metadata = _database.fetchElectionMetadata();

        // Generate tree if needed
        if(std::time(0) > metadata.electionend().epoch() && !_database.isSignedTreeGenerated()) {
                _logger.info("AsyncWork generating tree");

                // Generate tree
                std::vector<RecordedBallot> recordedBallots = _database.fetchRecordedBallotsSorted();
                SignedTree signedTree;
                treeGen(recordedBallots, signedTree.mutable_tree());

                // Sign tree
                std::string signedTreeSerialized;
                signedTree.SerializeToString(&signedTreeSerialized);
                std::string signature = SignMessage(
                        signedTreeSerialized,
                        _database.fetchVoteServerPrivateKey()
                );
                signedTree.mutable_signature()->set_signature(std::move(signature));

                // Save tree with ID 1
                _database.saveSignedTree(1, signedTree);
                
        }
}
