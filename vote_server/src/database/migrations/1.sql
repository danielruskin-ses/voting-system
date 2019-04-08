CREATE TABLE ELECTIONS(
        ID                              SERIAL PRIMARY KEY NOT NULL,
        START_TIME                      INT                NOT NULL,
        END_TIME                        INT                NOT NULL,
        ENABLED                         BOOLEAN            NOT NULL,
        ALLOW_WRITE_IN                  BOOLEAN            NOT NULL
);

CREATE TABLE VOTER_GROUPS(
        ID                              SERIAL PRIMARY KEY NOT NULL,
        NAME                            VARCHAR            NOT NULL
);

CREATE TABLE ELECTIONS_VOTER_GROUPS(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)       NOT NULL,
        VOTER_GROUP_ID                  INT REFERENCES VOTER_GROUPS(ID)    NOT NULL
);

CREATE TABLE VOTERS(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        VOTER_GROUP_ID                  INT REFERENCES VOTER_GROUPS(ID)    NOT NULL,
        FIRST_NAME                      VARCHAR                            NOT NULL,
        LAST_NAME                       VARCHAR                            NOT NULL,
        SMARTCARD_PUBLIC_KEY            BYTEA                              NOT NULL,
        REG_MATERIAL_HASH               BYTEA                              NOT NULL,
        REG_MATERIAL_IMG                BYTEA                              NOT NULL
);

CREATE TABLE CANDIDATES(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)       NOT NULL,
        FIRST_NAME                      VARCHAR                            NOT NULL,
        LAST_NAME                       VARCHAR                            NOT NULL
);

CREATE TABLE CAST_ENCRYPTED_BALLOTS(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        VOTER_ID                        INT REFERENCES VOTERS(ID)          NOT NULL,
        CAST_AT                         INT                                NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)       NOT NULL,
        
        CAST_COMMMAND_DATA              BYTEA                              NOT NULL,
        VOTER_SIGNATURE                 BYTEA                              NOT NULL
);

CREATE TABLE CAST_ENCRYPTED_BALLOT_ENTRIES(
        ID                              SERIAL PRIMARY KEY                                NOT NULL,
        CAST_ENCRYPTED_BALLOT_ID        INT REFERENCES CAST_ENCRYPTED_BALLOTS(ID)         NOT NULL,
        CANDIDATE_ID                    INT REFERENCES CANDIDATES(ID)                     NOT NULL,
        ENCRYPTED_VALUE                 BYTEA                                             NOT NULL
);

CREATE TABLE TALLIES(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)       NOT NULL
);

CREATE TABLE TALLY_ENTRIES(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        TALLY_ID                        INT REFERENCES TALLIES(ID)         NOT NULL,
        CANDIDATE_ID                    INT REFERENCES CANDIDATES(ID)      NOT NULL,
        ENCRYPTED_VALUE                 BYTEA                              NOT NULL,
        DECRYPTED_VALUE                 BYTEA                              NOT NULL,
        ENCRYPTION_R                    INT                                NOT NULL
);
