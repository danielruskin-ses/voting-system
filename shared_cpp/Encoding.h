#include <vector>

#include <pb_encode.h>
#include <pb_decode.h>

#include "gen_c/pb/shared.pb.h"
#include "shared_c/Definitions.h"

bool ByteTArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool StringEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool IntArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

template<typename T>
bool RepeatedMessageEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);

bool ByteTArrayDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

bool EncryptedBallotEntriesDecodeFunc(pb_istream_t *stream, const pb_field_t *field, void **arg);

template<typename T>
std::pair<bool, std::vector<BYTE_T>> encodeMessage(const pb_msgdesc_t* pb_fields, const T& message);
