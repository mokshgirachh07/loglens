#include "loglens/AnomalyDetector.h"

#include <algorithm>

namespace loglens {

void AnomalyDetector::add(const LogEntry& entry) {
    auto& times = recent_[entry.ip];

    // Drop requests that fell out of the window
    while (!times.empty() && entry.epoch - times.front() >= window_) {
        times.pop_front();
    }
    times.push_back(entry.epoch);

    if (times.size() > threshold_) {
        auto it = flagged_.find(entry.ip);
        if (it == flagged_.end()) {
            Alert a;
            a.ip = entry.ip;
            a.firstEpoch = entry.epoch;
            a.peakRequests = times.size();
            flagged_.emplace(entry.ip, a);
        } else {
            it->second.peakRequests = std::max(it->second.peakRequests, times.size());
        }
    }
}

std::vector<Alert> AnomalyDetector::alerts() const {
    std::vector<Alert> out;
    out.reserve(flagged_.size());
    for (const auto& kv : flagged_) out.push_back(kv.second);
    std::sort(out.begin(), out.end(), [](const Alert& a, const Alert& b) {
        if (a.peakRequests != b.peakRequests) return a.peakRequests > b.peakRequests;
        return a.ip < b.ip;
    });
    return out;
}

} 