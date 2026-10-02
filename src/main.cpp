#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "loglens/Analyzer.h"
#include "loglens/Reader.h"

namespace {

// 2'516'000'000 -> "2.34 GB"
std::string formatBytes(std::size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 4) {
        value /= 1024.0;
        ++unit;
    }
    std::ostringstream out;
    out << std::fixed << std::setprecision(unit == 0 ? 0 : 2) << value << ' ' << units[unit];
    return out.str();
}

void printTop(const std::string& title, const std::string& keyHeader,
              const loglens::RankedList& rows, std::size_t total) {
    // Make the key column as wide as the longest value
    std::size_t keyWidth = keyHeader.size();
    for (const auto& row : rows) keyWidth = std::max(keyWidth, row.first.size());
    const int kw = static_cast<int>(keyWidth);

    std::cout << title << '\n';
    std::cout << "  " << std::right << std::setw(3) << "#"
              << "  " << std::left << std::setw(kw) << keyHeader
              << "  " << std::right << std::setw(9) << "Requests"
              << "  " << std::setw(6) << "Share" << '\n';
    std::cout << "  " << std::string(3 + 2 + keyWidth + 2 + 9 + 2 + 6, '-') << '\n';

    std::cout << std::fixed << std::setprecision(1);
    int rank = 1;
    for (const auto& [key, count] : rows) {
        const double pct = 100.0 * static_cast<double>(count) / static_cast<double>(total);
        std::cout << "  " << std::right << std::setw(3) << rank++
                  << "  " << std::left << std::setw(kw) << key
                  << "  " << std::right << std::setw(9) << count
                  << "  " << std::setw(5) << pct << "%\n";
    }
    std::cout << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <logfile>\n";
        return 1;
    }

    loglens::Analyzer analyzer;
    const auto result = loglens::readLog(
        argv[1], [&](const loglens::LogEntry& e) { analyzer.add(e); });

    if (!result.opened) {
        std::cerr << "Error: could not open '" << argv[1] << "'\n";
        return 1;
    }
    if (result.parsed == 0) {
        std::cerr << "No valid log lines found.\n";
        return 1;
    }

    std::cout << "=== LogLens Report ===\n";
    std::cout << "Total lines : " << result.totalLines << '\n';
    std::cout << "Parsed      : " << result.parsed << '\n';
    std::cout << "Malformed   : " << result.malformed << '\n';
    std::cout << "Bytes served: " << formatBytes(analyzer.totalBytes()) << "\n\n";

    // Status codes (sorted by code)
    std::map<int, std::size_t> sorted(analyzer.statusCounts().begin(),
                                      analyzer.statusCounts().end());
    std::cout << "Status codes\n";
    std::cout << std::fixed << std::setprecision(1);
    for (const auto& [status, count] : sorted) {
        const double pct = 100.0 * static_cast<double>(count) / static_cast<double>(result.parsed);
        std::cout << "  " << status << "  " << std::right << std::setw(8) << count
                  << "  " << std::setw(5) << pct << "%\n";
    }
    std::cout << '\n';

    printTop("Top 10 IPs", "IP", analyzer.topIps(10), result.parsed);
    printTop("Top 10 URLs", "URL", analyzer.topUrls(10), result.parsed);

    std::cout << "Unique IPs  : " << analyzer.ipCounts().size() << '\n';
    return 0;
}