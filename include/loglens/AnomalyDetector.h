#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include "loglens/LogEntry.h"

namespace loglens {

struct Alert {
    std::string ip;
    std::int64_t firstEpoch = 0;   // when the burst was first detected
    std::size_t peakRequests = 0;  // most requests seen inside one window
};

class AnomalyDetector {
public:
    AnomalyDetector(std::size_t threshold = 100, std::int64_t windowSeconds = 60)
        : threshold_(threshold), window_(windowSeconds) {}

    void add(const LogEntry& entry);

    // Alerts sorted by peak requests, biggest first.
    std::vector<Alert> alerts() const;

private:
    std::size_t threshold_;
    std::int64_t window_;
    std::unordered_map<std::string, std::deque<std::int64_t>> recent_;
    std::unordered_map<std::string, Alert> flagged_;
};

}