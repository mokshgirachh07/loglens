#pragma once

#include <cstddef>
#include <functional>
#include <string>

#include "loglens/LogEntry.h"

namespace loglens {

struct ReadResult {
    bool opened = false;
    std::size_t totalLines = 0;
    std::size_t parsed = 0;
    std::size_t malformed = 0;
};

using EntryCallback = std::function<void(const LogEntry&)>;

// Streams the file line by line, parses each line, and calls onEntry for
// every valid entry. Malformed lines are counted and skipped.
ReadResult readLog(const std::string& path, const EntryCallback& onEntry);

} 