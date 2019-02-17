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
                std::vector<SignedRecordedBallot> signedRecordedBallots = _database.fetchSignedRecordedBallotsSorted();
                SignedTree signedTree;
                treeGen(signedRecordedBallots, signedTree.mutable_tree());

                // Sign tree
                SignMessage(
                        signedTree.tree(),
                        signedTree.mutable_signature(),
                        _database.fetchVoteServerPrivateKey()
                );

                // Save tree with ID 1
                _database.saveSignedTree(1, signedTree);
                
        }
}
