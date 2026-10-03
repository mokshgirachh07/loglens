#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <array>
#include "loglens/LogEntry.h"

namespace loglens {

// A list of (name, count) pairs, biggest count first.
using RankedList = std::vector<std::pair<std::string, std::size_t>>;

class Analyzer {
public:
    void add(const LogEntry& entry);
    const std::array<std::size_t, 24>& hourly() const { return hourly_; }
    const std::unordered_map<int, std::size_t>& statusCounts() const { return statusCounts_; }
    const std::unordered_map<std::string, std::size_t>& ipCounts() const { return ipCounts_; }
    std::size_t totalBytes() const { return totalBytes_; }

    RankedList topIps(std::size_t n) const;
    RankedList topUrls(std::size_t n) const;

private:
    std::unordered_map<int, std::size_t> statusCounts_;
    std::unordered_map<std::string, std::size_t> ipCounts_;
    std::unordered_map<std::string, std::size_t> urlCounts_;
    std::size_t totalBytes_ = 0;
    std::array<std::size_t, 24> hourly_{};
};

}