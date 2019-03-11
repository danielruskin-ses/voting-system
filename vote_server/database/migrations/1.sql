CREATE TABLE ELECTIONS(
        ID                              INT PRIMARY KEY NOT NULL,
        START_TIME                      INT             NOT NULL,
        END_TIME                        INT             NOT NULL,
        ENABLED                         BOOLEAN         NOT NULL,
        ALLOW_WRITE_IN                  BOOLEAN         NOT NULL
);

CREATE TABLE VOTER_GROUPS(
        ID                              INT PRIMARY KEY NOT NULL,
        NAME                            VARCHAR         NOT NULL
);

CREATE TABLE ELECTIONS_VOTER_GROUPS(
        ID                              INT PRIMARY KEY                 NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)    NOT NULL,
        VOTER_GROUP_ID                  INT REFERENCES VOTER_GROUPS(ID) NOT NULL
);

CREATE TABLE VOTERS(
        ID                              INT PRIMARY KEY                 NOT NULL,
        VOTER_GROUP_ID                  INT REFERENCES VOTER_GROUPS(ID) NOT NULL,
        FIRST_NAME                      VARCHAR                         NOT NULL,
        LAST_NAME                       VARCHAR                         NOT NULL,
        SMARTCARD_PUBLIC_KEY            BYTEA                           NOT NULL,
        REG_MATERIAL_HASH               BYTEA                           NOT NULL,
        REG_MATERIAL_IMG                BYTEA                           NOT NULL
);

CREATE TABLE CANDIDATES(
        ID                              INT PRIMARY KEY                 NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)    NOT NULL,
        FIRST_NAME                      VARCHAR                         NOT NULL,
        LAST_NAME                       VARCHAR                         NOT NULL
);

CREATE TABLE BALLOTS(
        ID                              INT PRIMARY KEY                 NOT NULL,
        SOURCE                          INT                             NOT NULL,
        VOTER_ID                        INT REFERENCES VOTERS(ID)       NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)    NOT NULL,
        ALT_SOURCE_HASH                 BYTEA                           NOT NULL,
        ALT_SOURCE_IMG                  BYTEA                           NOT NULL
);

CREATE TABLE BALLOT_ENTRIES(
        ID                              INT PRIMARY KEY                 NOT NULL,
        BALLOT_ID                       INT REFERENCES BALLOTS(ID)      NOT NULL,
        CANDIDATE_ID                    INT REFERENCES CANDIDATES(ID)   NOT NULL,
        ENCRYPTED_VALUE                 BYTEA                           NOT NULL
);

CREATE TABLE TALLIES(
        ID                              INT PRIMARY KEY                 NOT NULL,
        ELECTION_ID                     INT REFERENCES ELECTIONS(ID)    NOT NULL
);

CREATE TABLE TALLY_ENTRIES(
        ID                              INT PRIMARY KEY                 NOT NULL,
        TALLY_ID                        INT REFERENCES TALLIES(ID)      NOT NULL,
        CANDIDATE_ID                    INT REFERENCES CANDIDATES(ID)   NOT NULL,
        ENCRYPTED_VALUE                 BYTEA                           NOT NULL,
        DECRYPTED_VALUE                 BYTEA                           NOT NULL,
        ENCRYPTION_R                    INT                             NOT NULL
);
