
ƒ
shared.proto"A
	Signature
pubkey (Rpubkey
	signature (R	signature"W
	Candidate
id (Rid

first_name (	R	firstName
	last_name (	RlastName"å
Election
id (Rid$
start_time_utc (RstartTimeUtc 
end_time_utc (R
endTimeUtc
enabled (Renabled$
allow_write_in (RallowWriteIn;
authorized_voter_group_ids (RauthorizedVoterGroupIds9

candidates (2.Election.CandidatesEntryR
candidatesI
CandidatesEntry
key (Rkey 
value (2
.CandidateRvalue:8"0

VoterGroup
id (Rid
name (	Rname"¬
Voter
id (Rid
pubkey (Rpubkey
reg_hash (RregHash$
voter_group_id (RvoterGroupId

first_name (	R	firstName
	last_name (	RlastName"r
EncryptedBallotEntry
id (Rid!
candidate_id (RcandidateId'
encrypted_value (RencryptedValue"‚
EncryptedBallot
id (Rid
source (Rsource
voter_id (RvoterId
election_id (R
electionIdA
alternative_source_image_hash (RalternativeSourceImageHashf
encrypted_ballot_entries (2,.EncryptedBallot.EncryptedBallotEntriesEntryRencryptedBallotEntries`
EncryptedBallotEntriesEntry
key (Rkey+
value (2.EncryptedBallotEntryRvalue:8"r
DecryptedBallotEntry
id (Rid'
decrypted_value (RdecryptedValue!
encryption_r (RencryptionR"˜
PlaintextBallot;
encrypted_ballot (2.EncryptedBallotRencryptedBallotf
decrypted_ballot_entries (2,.PlaintextBallot.DecryptedBallotEntriesEntryRdecryptedBallotEntries`
DecryptedBallotEntriesEntry
key (Rkey+
value (2.DecryptedBallotEntryRvalue:8bproto3