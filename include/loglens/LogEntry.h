#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace loglens {

struct LogEntry {
    std::string ip;
    std::string timestamp;   // raw text, kept for reports
    std::int64_t epoch = 0;  // seconds since 1970 UTC (used for Day 6)
    int hour = 0;            // hour of day as written in the log
    std::string method;
    std::string url;
    int status = 0;
    std::size_t bytes = 0;
};

}  // namespace loglens