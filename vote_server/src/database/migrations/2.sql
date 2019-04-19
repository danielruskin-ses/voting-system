DROP TABLE tally_entries;

CREATE TABLE TALLY_ENTRIES(
        ID                              SERIAL PRIMARY KEY                 NOT NULL,
        TALLY_ID                        INT REFERENCES TALLIES(ID)         NOT NULL,
        CANDIDATE_ID                    INT REFERENCES CANDIDATES(ID)      NOT NULL,
        ENCRYPTED_VALUE                 BYTEA                              NOT NULL,
        DECRYPTED_VALUE                 INT                                NOT NULL,
        ENCRYPTION_R                    BYTEA                              NOT NULL
);
