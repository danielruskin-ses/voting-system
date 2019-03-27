#include <vector>

#include <pb_encode.h>
#include <pb_decode.h>

#include "shared_c/Definitions.h"

template<typename T>
std::pair<bool, std::vector<BYTE_T>> encodeMessage(const pb_field_t* pb_fields, const T& message) {
        std::vector<BYTE_T> vec; 

        size_t encodedSize = 0;
        bool resB = pb_get_encoded_size(&encodedSize, pb_fields, &message);
        if(!resB) {
                return {false, vec};
        }
        vec.resize(encodedSize + 2); // 2 bytes for size field
        pb_ostream_t buf = pb_ostream_from_buffer(&vec[0], encodedSize);
        resB = pb_encode_delimited(&buf, pb_fields, &message);
        if(!resB) {
                return {false, vec};
        }

        return {true, vec};
}
