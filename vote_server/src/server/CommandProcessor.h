#include <vector>
#include <pqxx/pqxx>

#include <pb_encode.h>
#include <pb_decode.h>
#include "gen_c/pb/shared.pb.h"

#include "shared_cpp/logger/Logger.h"
#include "../Config.h"
#include "shared_c/Definitions.h"

std::pair<bool, std::vector<BYTE_T>> processCommand(const std::vector<BYTE_T>& command, pqxx::connection& dbConn, Logger& logger, const Config& config);
