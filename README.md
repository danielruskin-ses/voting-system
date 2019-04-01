# Voting System

To setup:

1. Setup and install `libpqxx-dev`, `sqlpp11`, `sqlpp11-connector-postgresql`, WolfSSL on your machine
2. Install `nanopb` and put the path in the 'NANOPB_PATH` variable of the `vote_server` Makefile
3. Install Python and run `pip3 install psycopg2-binary`
4. Run `make` in `vote_server`
5. Setup a database and run `CREATE TABLE migrations(NAME VARCHAR PRIMARY KEY)`
