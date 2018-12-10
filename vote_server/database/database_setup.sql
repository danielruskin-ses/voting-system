CREATE TABLE IF NOT EXISTS CONFIG(
        ID                                     INT PRIMARY KEY NOT NULL,
        ELECTION_METADATA_SERIALIZED_DATA      BLOB            NOT NULL,
        VOTE_SERVER_PUBLIC_KEY                 BLOB            NOT NULL,
        VOTE_SERVER_PRIVATE_KEY                BLOB            NOT NULL
);
CREATE UNIQUE INDEX IF NOT EXISTS CONFIG_ID_IDX ON CONFIG(ID);

CREATE TABLE IF NOT EXISTS VOTER_DEVICES(
        ID         INT PRIMARY KEY NOT NULL,
        PUBLIC_KEY BLOB            NOT NULL
);
CREATE UNIQUE INDEX IF NOT EXISTS VOTER_DEVICES_ID_IDX ON VOTER_DEVICES(ID);

CREATE TABLE IF NOT EXISTS RECORDED_BALLOTS(
        VOTER_DEVICE_ID              INT PRIMARY KEY NOT NULL,
        SERIALIZED_DATA              BLOB            NOT NULL,

        FOREIGN KEY(VOTER_DEVICE_ID) REFERENCES VOTER_DEVICES(ID)
);
CREATE UNIQUE INDEX IF NOT EXISTS RECORDED_BALLOTS_ID_IDX ON RECORDED_BALLOTS(VOTER_DEVICE_ID);

CREATE TABLE IF NOT EXISTS SIGNED_TREES(
        ID INT PRIMARY  KEY  NOT NULL,
        SERIALIZED_DATA BLOB NOT NULL
)
CREATE UNIQUE INDEX IF NOT EXISTS SIGNED_TREES_ID_IDX ON SIGNED_TREES(ID);
