#include "loglens/Analyzer.h"

#include <algorithm>

namespace loglens {

namespace {

// Returns the n biggest entries, sorted from biggest to smallest.
RankedList topN(const std::unordered_map<std::string, std::size_t>& counts, std::size_t n) {
    RankedList items(counts.begin(), counts.end());
    n = std::min(n, items.size());

    std::partial_sort(items.begin(), items.begin() + static_cast<std::ptrdiff_t>(n), items.end(),
                      [](const auto& a, const auto& b) {
                          if (a.second != b.second) return a.second > b.second;
                          return a.first < b.first;  // tie-break: alphabetical
                      });

    items.resize(n);
    return items;
}

}  // namespace

void Analyzer::add(const LogEntry& entry) {
    ++statusCounts_[entry.status];
    ++ipCounts_[entry.ip];
    ++urlCounts_[entry.url];
    totalBytes_ += entry.bytes;
}

RankedList Analyzer::topIps(std::size_t n) const { return topN(ipCounts_, n); }
RankedList Analyzer::topUrls(std::size_t n) const { return topN(urlCounts_, n); }

}