#include "loglens/Report.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace loglens {

namespace {

std::string formatBytes(std::size_t bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    double value = static_cast<double>(bytes);
    int unit = 0;
    while (value >= 1024.0 && unit < 4) {
        value /= 1024.0;
        ++unit;
    }
    std::ostringstream o;
    o << std::fixed << std::setprecision(unit == 0 ? 0 : 2) << value << ' ' << units[unit];
    return o.str();
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

    std::ostringstream o;
    o << (y + (m <= 2)) << '-' << std::setfill('0') << std::setw(2) << m << '-'
      << std::setw(2) << d << ' ' << std::setw(2) << secs / 3600 << ':'
      << std::setw(2) << (secs % 3600) / 60 << ':' << std::setw(2) << secs % 60;
    return o.str();
}

void printTop(std::ostream& out, const std::string& title, const std::string& keyHeader,
              const RankedList& rows, std::size_t total) {
    std::size_t keyWidth = keyHeader.size();
    for (const auto& row : rows) keyWidth = std::max(keyWidth, row.first.size());
    const int kw = static_cast<int>(keyWidth);

    out << title << '\n';
    out << "  " << std::right << std::setw(3) << "#"
        << "  " << std::left << std::setw(kw) << keyHeader
        << "  " << std::right << std::setw(9) << "Requests"
        << "  " << std::setw(6) << "Share" << '\n';
    out << "  " << std::string(3 + 2 + keyWidth + 2 + 9 + 2 + 6, '-') << '\n';

    out << std::fixed << std::setprecision(1);
    int rank = 1;
    for (const auto& [key, count] : rows) {
        const double pct = 100.0 * static_cast<double>(count) / static_cast<double>(total);
        out << "  " << std::right << std::setw(3) << rank++
            << "  " << std::left << std::setw(kw) << key
            << "  " << std::right << std::setw(9) << count
            << "  " << std::setw(5) << pct << "%\n";
    }
    out << '\n';
}

void printHourly(std::ostream& out, const std::array<std::size_t, 24>& hourly) {
    const std::size_t peak = *std::max_element(hourly.begin(), hourly.end());
    if (peak == 0) return;

    const std::size_t maxBar = 40;
    out << "Traffic by hour\n";
    for (std::size_t h = 0; h < hourly.size(); ++h) {
        std::size_t len = hourly[h] * maxBar / peak;
        if (hourly[h] > 0 && len == 0) len = 1;
        out << "  " << std::setfill('0') << std::right << std::setw(2) << h
            << std::setfill(' ') << ":00 | " << std::string(len, '#')
            << ' ' << hourly[h] << '\n';
    }
    out << '\n';
}

// CSV fields containing commas or quotes must be wrapped in quotes
std::string csvEscape(const std::string& s) {
    if (s.find_first_of(",\"\n") == std::string::npos) return s;
    std::string r = "\"";
    for (char c : s) {
        if (c == '"') r += '"';
        r += c;
    }
    return r + "\"";
}

}  // namespace

void writeTextReport(std::ostream& out, const ReportData& d) {
    out << "=== LogLens Report ===\n";
    out << "Total lines : " << d.read.totalLines << '\n';
    out << "Parsed      : " << d.read.parsed << '\n';
    out << "Malformed   : " << d.read.malformed << '\n';
    out << "Matched     : " << d.matched << "  (after filters)\n";

    if (d.matched == 0) {
        out << "\nNo requests matched your filters.\n";
        return;
    }

    out << "Bytes served: " << formatBytes(d.analyzer.totalBytes()) << "\n\n";

    std::map<int, std::size_t> sorted(d.analyzer.statusCounts().begin(),
                                      d.analyzer.statusCounts().end());
    out << "Status codes\n";
    out << std::fixed << std::setprecision(1);
    for (const auto& [status, count] : sorted) {
        const double pct = 100.0 * static_cast<double>(count) / static_cast<double>(d.matched);
        out << "  " << status << "  " << std::right << std::setw(8) << count
            << "  " << std::setw(5) << pct << "%\n";
    }
    out << '\n';

    printTop(out, "Top IPs", "IP", d.analyzer.topIps(d.top), d.matched);
    printTop(out, "Top URLs", "URL", d.analyzer.topUrls(d.top), d.matched);
    printHourly(out, d.analyzer.hourly());

    out << "Anomaly detection (more than " << d.threshold << " requests in 60s)\n";
    const auto alerts = d.detector.alerts();
    if (alerts.empty()) {
        out << "  none found\n\n";
    } else {
        for (const auto& a : alerts) {
            out << "  ! " << a.ip << "  peak " << a.peakRequests
                << " requests/60s, first flagged at "
                << formatEpoch(a.firstEpoch, 5 * 3600 + 30 * 60) << " (+0530)\n";
        }
        out << '\n';
    }

    out << "Unique IPs  : " << d.analyzer.ipCounts().size() << '\n';
}

void writeCsvReport(std::ostream& out, const ReportData& d) {
    out << "section,key,value\n";
    out << "summary,total_lines," << d.read.totalLines << '\n';
    out << "summary,parsed," << d.read.parsed << '\n';
    out << "summary,malformed," << d.read.malformed << '\n';
    out << "summary,matched," << d.matched << '\n';
    out << "summary,bytes," << d.analyzer.totalBytes() << '\n';
    out << "summary,unique_ips," << d.analyzer.ipCounts().size() << '\n';

    std::map<int, std::size_t> sorted(d.analyzer.statusCounts().begin(),
                                      d.analyzer.statusCounts().end());
    for (const auto& [status, count] : sorted) out << "status," << status << ',' << count << '\n';

    for (const auto& [ip, count] : d.analyzer.topIps(d.top))
        out << "top_ip," << csvEscape(ip) << ',' << count << '\n';
    for (const auto& [url, count] : d.analyzer.topUrls(d.top))
        out << "top_url," << csvEscape(url) << ',' << count << '\n';

    const auto& hourly = d.analyzer.hourly();
    for (std::size_t h = 0; h < hourly.size(); ++h) out << "hour," << h << ',' << hourly[h] << '\n';

    for (const auto& a : d.detector.alerts())
        out << "alert," << csvEscape(a.ip) << ',' << a.peakRequests << '\n';
}

} 