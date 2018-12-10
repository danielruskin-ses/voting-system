#include "AsyncWork.h"
#include "Crypto.shared.h"
#include "TreeGen.shared.h"

AsyncWork::AsyncWork(const Logger& logger, Database& database) : _logger(logger), _database(database), _shouldExit(false) {
        _thread = std::thread(&AsyncWork::loop, this);
}

AsyncWork::~AsyncWork() {
        _shouldExit = true;
        
        if(_thread.joinable()) {
                _thread.join();
        }
}

void AsyncWork::loop() {
        _logger.info("AsyncWork thread started");

        std::chrono::seconds sleepDuration(10);
        while(!_shouldExit) {
                _logger.info("AsyncWork loop iteration");

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

                // Sleep for 10s
                std::this_thread::sleep_for(sleepDuration);
        }
}
