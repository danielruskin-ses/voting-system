#include <vector>

#include <pb_encode.h>
#include <pb_decode.h>
#include "c/pb/shared.pb.h"

std::vector<uint8_t> processCommand(const std::vector<uint8_t>& command);
