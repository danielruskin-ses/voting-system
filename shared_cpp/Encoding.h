#include <vector>

#include <pb_encode.h>
#include <pb_decode.h>

#include "gen_c/pb/shared.pb.h"
#include "shared_c/Definitions.h"

bool ByteTArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool StringEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool IntArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool RepeatedCandidateEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
bool RepeatedTallyEntryEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
bool RepeatedElectionEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
bool RepeatedEncryptedBallotEntryEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool StringDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool IntArrayDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool CandidatesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool TallyEntriesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool ByteTArrayDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool EncryptedBallotEntriesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool ElectionsDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

template<typename T>
std::pair<bool, std::vector<BYTE_T>> encodeMessage(const pb_msgdesc_t* pb_fields, const T& message);
