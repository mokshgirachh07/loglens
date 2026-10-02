#pragma once

#include <cstddef>
#include <string>

namespace loglens {

struct LogEntry {
    std::string ip;
    std::string timestamp;   // kept as raw text for now; we parse it on Day 5
    std::string method;
    std::string url;
    int status = 0;
    std::size_t bytes = 0;
};

} 