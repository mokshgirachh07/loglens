#include "loglens/Parser.h"
#include "loglens/Timestamp.h"
#include <regex>

namespace loglens {

std::optional<LogEntry> parseLine(const std::string& line) {
    // Example line:
    // 10.0.0.14 - - [30/Sep/2026:10:15:32 +0530] "GET /about HTTP/1.1" 200 8123
    static const std::regex pattern(
        R"(^(\S+) \S+ \S+ \[([^\]]+)\] "(\S+) (\S+) [^"]*" (\d{3}) (\d+|-)\s*$)");

    std::smatch m;
    if (!std::regex_match(line, m, pattern)) {
        return std::nullopt;
    }

    const auto time = parseTimestamp(m[2].str());
    if (!time) {
        return std::nullopt;  // bad timestamp = malformed line
    }

    LogEntry entry;
    entry.ip        = m[1].str();
    entry.timestamp = m[2].str();
    entry.epoch     = time->epoch;
    entry.hour      = time->hour;
    entry.method    = m[3].str();
    entry.url       = m[4].str();
    entry.status    = std::stoi(m[5].str());
    entry.bytes     = (m[6].str() == "-") ? 0 : std::stoull(m[6].str());
    return entry;
}

} 