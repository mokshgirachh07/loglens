#include <algorithm>
#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "loglens/Analyzer.h"
#include "loglens/Reader.h"
#include "loglens/AnomalyDetector.h"

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

void printHourly(const std::array<std::size_t, 24>& hourly) {
    const std::size_t peak = *std::max_element(hourly.begin(), hourly.end());
    if (peak == 0) return;

    const std::size_t maxBar = 40;
    std::cout << "Traffic by hour\n";
    for (std::size_t h = 0; h < hourly.size(); ++h) {
        std::size_t len = hourly[h] * maxBar / peak;
        if (hourly[h] > 0 && len == 0) len = 1;  // never hide non-empty hours

        std::cout << "  " << std::setfill('0') << std::right << std::setw(2) << h
                  << std::setfill(' ') << ":00 | " << std::string(len, '#')
                  << ' ' << hourly[h] << '\n';
    }
    std::cout << '\n';
}
std::string formatEpoch(std::int64_t epoch, int offsetSeconds) {
    std::int64_t t = epoch + offsetSeconds;
    std::int64_t days = t / 86400;
    std::int64_t secs = t % 86400;
    if (secs < 0) { secs += 86400; --days; }

    days += 719468;
    const std::int64_t era = (days >= 0 ? days : days - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(days - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const std::int64_t y = static_cast<std::int64_t>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;
    const unsigned m = mp < 10 ? mp + 3 : mp - 9;

    std::ostringstream out;
    out << (y + (m <= 2)) << '-' << std::setfill('0') << std::setw(2) << m << '-'
        << std::setw(2) << d << ' ' << std::setw(2) << secs / 3600 << ':'
        << std::setw(2) << (secs % 3600) / 60 << ':' << std::setw(2) << secs % 60;
    return out.str();
}

void printAlerts(const std::vector<loglens::Alert>& alerts, std::size_t threshold) {
    std::cout << "Anomaly detection (more than " << threshold << " requests in 60s)\n";
    if (alerts.empty()) {
        std::cout << "  none found\n\n";
        return;
    }
    for (const auto& a : alerts) {
        std::cout << "  ! " << a.ip << "  peak " << a.peakRequests
                  << " requests/60s, first flagged at "
                  << formatEpoch(a.firstEpoch, 5 * 3600 + 30 * 60) << " (+0530)\n";
    }
    std::cout << '\n';
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <logfile>\n";
        return 1;
    }

    const std::size_t threshold = 100;
    loglens::Analyzer analyzer;
    loglens::AnomalyDetector detector(threshold, 60);
    const auto result = loglens::readLog(argv[1], [&](const loglens::LogEntry& e) {
        analyzer.add(e);
        detector.add(e);
    });

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
    printHourly(analyzer.hourly());
    printAlerts(detector.alerts(), threshold);

    std::cout << "Unique IPs  : " << analyzer.ipCounts().size() << '\n';
    return 0;
}