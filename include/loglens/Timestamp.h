#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace loglens {

struct ParsedTime {
    std::int64_t epoch = 0;  // seconds since 1970-01-01 UTC
    int hour = 0;            // hour as written in the log (0-23)
};

// Parses "30/Sep/2026:10:15:32 +0530". Returns nullopt if malformed.
std::optional<ParsedTime> parseTimestamp(const std::string& s);

}