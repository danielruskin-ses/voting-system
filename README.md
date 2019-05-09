# Voting System

To setup:

1. Setup and install `libgrypt-dev`, `libgmp3-dev`, `libpqxx-dev`, `sqlpp11`, `sqlpp11-connector-postgresql`, `libpaillier`, WolfSSL, LibTMCG on your machine (make sure to use the libraries in the `lib` folder, they are modified for our use)
2. Install `nanopb` and put the path in the 'NANOPB_PATH` variable of the `vote_server` Makefile
3. Install Python and run `pip3 install psycopg2-binary`
4. Run `make` in `vote_server`
5. Setup a database and run `CREATE TABLE migrations(NAME VARCHAR PRIMARY KEY)`
