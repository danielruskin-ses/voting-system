#include "Encoding.h"

#include <string>

bool ByteTArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;
        
        const std::vector<BYTE_T>& argReal = *((const std::vector<BYTE_T>*) arg);
        return pb_encode_string(stream, &(argReal[0]), argReal.size());
}

bool StringEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        if (!pb_encode_tag_for_field(stream, field))
            return false;
        
        const std::string& argReal = *((const std::string*) arg);
        return pb_encode_string(stream, (const BYTE_T*) argReal.c_str(), argReal.size() + 1);
}

bool IntArrayEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        const std::vector<int>& argReal = *((const std::vector<int>*) arg);

        for(int i = 0; i < argReal.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field))
                        return false;

                if (!pb_encode_varint(stream, argReal[i]))
                        return false;
        }
}

template<typename T>
bool RepeatedMessageEncodeFunc(pb_ostream_t *stream, const pb_field_t *field, void * const *arg) {
        auto argReal = (const std::pair<const pb_msgdesc_t*, const std::vector<T>*>*) arg;
        const pb_msgdesc_t* fields = argReal->first;
        const std::vector<T>& arr = *(argReal->second);
        
        for(int i = 0; i < arr.size(); i++) {
                if (!pb_encode_tag_for_field(stream, field))
                        return false;

                if (!pb_encode(stream, fields, &(arr[i])))
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

template<typename T>
std::pair<bool, std::vector<BYTE_T>> encodeMessage(const pb_msgdesc_t* pb_fields, const T& message) {
        std::vector<BYTE_T> vec; 

        size_t encodedSize = 0;
        bool resB = pb_get_encoded_size(&encodedSize, pb_fields, &message);
        if(!resB) {
                return {false, vec};
        }
        vec.resize(encodedSize + 2); // 2 bytes for size field
        pb_ostream_t buf = pb_ostream_from_buffer(&vec[0], vec.size());
        resB = pb_encode_delimited(&buf, pb_fields, &message);
        if(!resB) {
                return {false, vec};
        }

        return {true, vec};
}

// Explicit instantiation.  Required to fix linker errors arising from template fctns in cpp file.
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<Response>(const pb_msgdesc_t* pb_fields, const Response& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<CastEncryptedBallot>(const pb_msgdesc_t* pb_fields, const CastEncryptedBallot& message);
template std::pair<bool, std::vector<BYTE_T>> encodeMessage<Elections>(const pb_msgdesc_t* pb_fields, const Elections& message);
template bool RepeatedMessageEncodeFunc<EncryptedBallotEntry>(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
template bool RepeatedMessageEncodeFunc<Candidate>(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);
template bool RepeatedMessageEncodeFunc<Election>(pb_ostream_t *stream, const pb_field_t *field, void * const *arg);