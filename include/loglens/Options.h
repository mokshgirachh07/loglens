#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace loglens {

struct Options {
    std::string path;
    std::size_t top = 10;
    std::size_t threshold = 100;
    std::optional<int> status;           // --status 404
    std::optional<std::string> ip;       // --ip 10.0.0.5
    std::optional<std::int64_t> fromEpoch;  // --from 2026-09-30 (00:00:00, log timezone)
    std::optional<std::int64_t> toEpoch;    // --to 2026-09-30 (end of that day)
    bool help = false;
    std::string outPath;   // --out report.txt
    std::string csvPath;   // --csv report.csv
};

// Parses argv. On error returns false and fills `error`.
bool parseOptions(int argc, char* argv[], Options& opts, std::string& error);

void printUsage(const char* program);

} 