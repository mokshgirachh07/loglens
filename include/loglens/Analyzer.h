#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

#include "loglens/LogEntry.h"

namespace loglens {

class Analyzer {
public:
    void add(const LogEntry& entry);

    const std::unordered_map<int, std::size_t>& statusCounts() const { return statusCounts_; }
    const std::unordered_map<std::string, std::size_t>& ipCounts() const { return ipCounts_; }

private:
    std::unordered_map<int, std::size_t> statusCounts_;
    std::unordered_map<std::string, std::size_t> ipCounts_;
};

}