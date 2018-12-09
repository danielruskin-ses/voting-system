# Vote Server

This folder contains the source code for the vote server, as well as a client for debugging.

# Setup

1. Install [GRPC C++](https://grpc.io/docs/quickstart/cpp.html)
2. Install [CryptoPP](https://www.cryptopp.com/)
3. Install [SQLite3](https://www.sqlite.org/index.html) for C++
4. Run `make`

# Testing
1. Start the vote server with `./vote_server`
2. Run `./vote_server_debug create_config` to create and save sample election metadata
3. Run `./vote_server_debug ftech_election_metadata` to test the fetch election metadata function
4. Run `./vote_server_debug create_voter_device <choose an integer ID>` to create and save a sample voter device
5. Run `./vote_server_debug cast_proposed_ballot <choose an integer ID> <Private key from step 3>` to test the cast ballot functionality
