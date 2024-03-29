#include "Encoding.h"

#include <string>
#include <tuple>
#include <iostream>

bool ByteTArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;
        
        const std::vector<BYTE_T>& argReal = *((const std::vector<BYTE_T>* const) *arg);
        return pb_encode_string(stream, &(argReal[0]), argReal.size());
}

bool StringEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;

        const std::string& argReal = *((const std::string* const) *arg);
        return pb_encode_string(stream, (const BYTE_T*) argReal.c_str(), argReal.size() + 1);
}

bool IntArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<int>& argReal = *((const std::vector<int>* const) *arg);

        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field)) {
                        return false;
                }

                if (!pb_encode_varint(stream, argReal[i])) {
                        return false;
                }
        }

        return true;
}

// TODO: DRY up these funcs
bool RepeatedCandidateEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<Candidate>& argReal = *((const std::vector<Candidate>* const) *arg);
        
        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field)) {
                        return false;
                }

                if (!pb_encode_submessage(stream, Candidate_fields, &(argReal[i]))) {
                        return false;
                }
        }

        return true;
}
bool RepeatedTallyEntryEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<TallyEntry>& argReal = *((const std::vector<TallyEntry>* const) *arg);
        
        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field)) {
                        return false;
                }

                if (!pb_encode_submessage(stream, TallyEntry_fields, &(argReal[i]))) {
                        return false;
                }
        }

        return true;
}
bool RepeatedElectionEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<Election>& argReal = *((const std::vector<Election>* const) *arg);
        
        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field)) {
                        return false;
                }

                if (!pb_encode_submessage(stream, Election_fields, &(argReal[i]))) {
                        return false;
                }
        }

        return true;
}
bool RepeatedEncryptedBallotEntryEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<EncryptedBallotEntry>& argReal = *((const std::vector<EncryptedBallotEntry>* const) *arg);

        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field))
                        return false;

                if (!pb_encode_submessage(stream, EncryptedBallotEntry_fields, &(argReal[i])))
                        return false;
        }

        return true;
}
bool RepeatedCastEncryptedBallotEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<CastEncryptedBallot>& argReal = *((const std::vector<CastEncryptedBallot>* const) *arg);
        
        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field))
                        return false;

                if (!pb_encode_submessage(stream, CastEncryptedBallot_fields, &(argReal[i])))
                        return false;
        }

        return true;
}
bool RepeatedVoterEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<Voter>& argReal = *((const std::vector<Voter>* const) *arg);
        
        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field))
                        return false;

                if (!pb_encode_submessage(stream, Voter_fields, &(argReal[i])))
                        return false;
        }

        return true;
}

bool ByteTArrayDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::vector<BYTE_T>*) *arg;
        argReal->resize(stream->bytes_left);

        if (!pb_read(stream, &((*argReal)[0]), stream->bytes_left))
                return false;

        return true;
}

bool EncryptedBallotEntriesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::pair<std::vector<EncryptedBallotEntry>*, std::vector<std::vector<BYTE_T>>*>*) *arg;
        std::vector<EncryptedBallotEntry>& argRealFirst = *(argReal->first);
        std::vector<std::vector<BYTE_T>>& argRealSecond = *(argReal->second);

        argRealFirst.resize(argRealFirst.size() + 1);
        argRealSecond.resize(argRealSecond.size() + 1);

        EncryptedBallotEntry* ebe = &(argRealFirst[argRealFirst.size() - 1]);
        ebe->encrypted_value.arg = &(argRealSecond[argRealSecond.size() - 1]); 
        ebe->encrypted_value.funcs.decode = ByteTArrayDecodeFunc;

        if (!pb_decode(stream, EncryptedBallotEntry_fields, ebe))
            return false;

        return true;
}

bool StringDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::string*) *arg;
        
        char buffer[stream->bytes_left];
        if(!pb_read(stream, (BYTE_T*) buffer, stream->bytes_left))
                return false;

        *argReal = std::string(buffer, sizeof(buffer));

        return true;
}

bool IntArrayDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::vector<int>*) *arg;

        argReal->resize(argReal->size() + 1);

        uint64_t value;
        if (!pb_decode_varint(stream, &value))
                return false;
        
        (*argReal)[argReal->size() - 1] = value;

        return true;
}

bool CandidatesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::vector<std::tuple<Candidate, std::string, std::string>>*) *arg;

        argReal->resize(argReal->size() + 1);

        auto& lastElem = (*argReal)[argReal->size() - 1];

        std::get<0>(lastElem).first_name.arg = &(std::get<1>(lastElem));
        std::get<0>(lastElem).first_name.funcs.decode = StringDecodeFunc; 
        std::get<0>(lastElem).last_name.arg = &(std::get<2>(lastElem));
        std::get<0>(lastElem).last_name.funcs.decode = StringDecodeFunc;

        if (!pb_decode(stream, Candidate_fields, &(std::get<0>(lastElem)))) {
                return false;
        }

        return true;
}

bool TallyEntriesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::vector<std::tuple<TallyEntry, std::vector<BYTE_T>, std::vector<BYTE_T>>>*) *arg;

        argReal->resize(argReal->size() + 1);

        auto& lastElem = (*argReal)[argReal->size() - 1];

        std::get<0>(lastElem).encrypted_value.arg = &(std::get<1>(lastElem));
        std::get<0>(lastElem).encrypted_value.funcs.decode = ByteTArrayDecodeFunc; 
        std::get<0>(lastElem).encryption_r.arg = &(std::get<2>(lastElem));
        std::get<0>(lastElem).encryption_r.funcs.decode = ByteTArrayDecodeFunc;

        if (!pb_decode(stream, TallyEntry_fields, &(std::get<0>(lastElem)))) {
                return false;
        }

        return true;
}

bool CastEncryptedBallotsDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::tuple<std::vector<CastEncryptedBallot>*, std::vector<std::pair<std::vector<BYTE_T>, std::vector<BYTE_T>>>*, std::vector<std::vector<EncryptedBallotEntry>>*, std::vector<std::vector<std::vector<BYTE_T>>>*>*) *arg;
        std::vector<CastEncryptedBallot>& argRealFirst = *(std::get<0>(*argReal));
        std::vector<std::pair<std::vector<BYTE_T>, std::vector<BYTE_T>>>& argRealSecond = *(std::get<1>(*argReal));
        std::vector<std::vector<EncryptedBallotEntry>>& argRealThird = *(std::get<2>(*argReal));
        std::vector<std::vector<std::vector<BYTE_T>>>& argRealFourth = *(std::get<3>(*argReal));

        argRealFirst.resize(argRealFirst.size() + 1);
        argRealSecond.resize(argRealSecond.size() + 1);
        argRealThird.resize(argRealThird.size() + 1);
        argRealFourth.resize(argRealFourth.size() + 1);

        CastEncryptedBallot* ceb = &(argRealFirst.back());
        ceb->cast_command_data.arg = &(argRealSecond.back().first);
        ceb->cast_command_data.funcs.decode = ByteTArrayDecodeFunc;
        ceb->voter_signature.arg = &(argRealSecond.back().second);
        ceb->voter_signature.funcs.decode = ByteTArrayDecodeFunc;

        std::pair<std::vector<EncryptedBallotEntry>*, std::vector<std::vector<BYTE_T>>*> argsInner = {&(argRealThird.back()), &(argRealFourth.back())};
        ceb->encrypted_ballot.encrypted_ballot_entries.arg = &argsInner;
        ceb->encrypted_ballot.encrypted_ballot_entries.funcs.decode = EncryptedBallotEntriesDecodeFunc;

        if (!pb_decode(stream, CastEncryptedBallot_fields, ceb)) {
                return false;
        }

        return true;
}

bool ElectionsDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg) {
        auto argReal = (std::tuple<std::vector<Election>*, std::vector<std::vector<int>>*, std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>>*, std::vector<std::vector<std::tuple<TallyEntry, std::vector<BYTE_T>, std::vector<BYTE_T>>>>*>*) *arg;
        std::vector<Election>& argRealFirst = *(std::get<0>(*argReal));
        std::vector<std::vector<int>>& argRealSecond = *(std::get<1>(*argReal));
        std::vector<std::vector<std::tuple<Candidate, std::string, std::string>>>& argRealThird = *(std::get<2>(*argReal));
        std::vector<std::vector<std::tuple<TallyEntry, std::vector<BYTE_T>, std::vector<BYTE_T>>>>& argRealFourth = *(std::get<3>(*argReal));

        argRealFirst.resize(argRealFirst.size() + 1);
        argRealSecond.resize(argRealSecond.size() + 1);
        argRealThird.resize(argRealThird.size() + 1);
        argRealFourth.resize(argRealFourth.size() + 1);

        Election* election = &(argRealFirst[argRealFirst.size() - 1]);
        election->authorized_voter_group_ids.arg = &(argRealSecond[argRealSecond.size() - 1]);
        election->authorized_voter_group_ids.funcs.decode = IntArrayDecodeFunc;
        election->candidates.arg = &(argRealThird[argRealThird.size() - 1]);
        election->candidates.funcs.decode = CandidatesDecodeFunc;
        election->tally.tally_entries.arg = &(argRealFourth[argRealFourth.size() - 1]);
        election->tally.tally_entries.funcs.decode = TallyEntriesDecodeFunc;

        if (!pb_decode(stream, Election_fields, election)) {
                return false;
        }

        return true;
}

template<typename T>
std::pair<bool, std::vector<BYTE_T>> encodeMessage(const pb_msgdesc_t* pb_fields, const T& message) {
        std::vector<BYTE_T> vec; 

        size_t encodedSize = 0;
        bool resB = pb_get_encoded_size(&encodedSize, pb_fields, &message);
        if(!resB) {
                return {false, vec};
        }
        vec.resize(encodedSize); // Extra bytes for size field
        pb_ostream_t buf = pb_ostream_from_buffer(&vec[0], vec.size());
        resB = pb_encode(&buf, pb_fields, &message);
        if(!resB) {
                return {false, vec};
        }

        return {true, vec};
}

// Explicit instantiation.  Required to fix linker errors arising from template fctns in cpp file.
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<Response>(const pb_msgdesc_t* pb_fields, const Response& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<Command>(const pb_msgdesc_t* pb_fields, const Command& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<EncryptedBallot>(const pb_msgdesc_t* pb_fields, const EncryptedBallot& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<PaginationMetadata>(const pb_msgdesc_t* pb_fields, const PaginationMetadata& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<CastEncryptedBallot>(const pb_msgdesc_t* pb_fields, const CastEncryptedBallot& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<CastEncryptedBallots>(const pb_msgdesc_t* pb_fields, const CastEncryptedBallots& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<CastEncryptedBallotsRequest>(const pb_msgdesc_t* pb_fields, const CastEncryptedBallotsRequest& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<Voters>(const pb_msgdesc_t* pb_fields, const Voters& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<Elections>(const pb_msgdesc_t* pb_fields, const Elections& message);
