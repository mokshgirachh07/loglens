#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>

#include "loglens/Analyzer.h"
#include "loglens/Reader.h"

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
    std::cout << "Malformed   : " << result.malformed << "\n\n";

    // unordered_map has no order, so copy into a std::map to print sorted
    std::map<int, std::size_t> sorted(analyzer.statusCounts().begin(),
                                      analyzer.statusCounts().end());

    std::cout << "Status codes\n";
    std::cout << std::fixed << std::setprecision(1);
    for (const auto& [status, count] : sorted) {
        const double pct = 100.0 * static_cast<double>(count) / static_cast<double>(result.parsed);
        std::cout << "  " << status << "  " << std::setw(8) << count
                  << "  " << std::setw(5) << pct << "%\n";
    }

    const auto& ips = analyzer.ipCounts();
    const auto busiest = std::max_element(
        ips.begin(), ips.end(),
        [](const auto& a, const auto& b) { return a.second < b.second; });

    std::cout << "\nUnique IPs  : " << ips.size() << '\n';
    std::cout << "Busiest IP  : " << busiest->first << " (" << busiest->second << " requests)\n";
    return 0;
}