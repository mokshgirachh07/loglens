#include "loglens/Options.h"

#include <iostream>
#include <stdexcept>

#include "loglens/Timestamp.h"

namespace loglens {

namespace {

// "2026-09-30" -> epoch of 00:00:00 in the log's timezone (+0530)
std::optional<std::int64_t> parseDate(const std::string& s, bool endOfDay) {
    if (s.size() != 10 || s[4] != '-' || s[7] != '-') return std::nullopt;
    static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    try {
        const int month = std::stoi(s.substr(5, 2));
        if (month < 1 || month > 12) return std::nullopt;
        const std::string text = s.substr(8, 2) + "/" + months[month - 1] + "/" +
                                 s.substr(0, 4) + (endOfDay ? ":23:59:59" : ":00:00:00") +
                                 " +0530";
        const auto t = parseTimestamp(text);
        if (!t) return std::nullopt;
        return t->epoch;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

}  // namespace

bool parseOptions(int argc, char* argv[], Options& opts, std::string& error) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        // Flags that take a value: grab the next argument
        auto next = [&](std::string& value) {
            if (i + 1 >= argc) {
                error = "missing value for " + arg;
                return false;
            }
            value = argv[++i];
            return true;
        };

        std::string value;
        try {
            if (arg == "--help" || arg == "-h") {
                opts.help = true;
            } else if (arg == "--top") {
                if (!next(value)) return false;
                const long n = std::stol(value);
                if (n < 1) { error = "--top must be at least 1"; return false; }
                opts.top = static_cast<std::size_t>(n);
            } else if (arg == "--threshold") {
                if (!next(value)) return false;
                const long n = std::stol(value);
                if (n < 1) { error = "--threshold must be at least 1"; return false; }
                opts.threshold = static_cast<std::size_t>(n);
            } else if (arg == "--status") {
                if (!next(value)) return false;
                opts.status = std::stoi(value);
            } else if (arg == "--ip") {
                if (!next(value)) return false;
                opts.ip = value;
            } else if (arg == "--from") {
                if (!next(value)) return false;
                opts.fromEpoch = parseDate(value, false);
                if (!opts.fromEpoch) { error = "bad date for --from (use YYYY-MM-DD)"; return false; }
            } else if (arg == "--to") {
                if (!next(value)) return false;
                opts.toEpoch = parseDate(value, true);
                if (!opts.toEpoch) { error = "bad date for --to (use YYYY-MM-DD)"; return false; }
            } else if (arg == "--out") {
                if (!next(value)) return false;
                opts.outPath = value;
            } else if (arg == "--csv") {
                if (!next(value)) return false;
                opts.csvPath = value;
            } else if (!arg.empty() && arg[0] == '-') {
                error = "unknown option " + arg;
                return false;
            } else if (opts.path.empty()) {
                opts.path = arg;
            } else {
                error = "unexpected extra argument " + arg;
                return false;
            }
        } catch (const std::exception&) {
            error = "invalid number for " + arg;
            return false;
        }
    }

    if (!opts.help && opts.path.empty()) {
        error = "no log file given";
        return false;
    }
    return true;
}

void printUsage(const char* program) {
    std::cout <<
        "LogLens - web server log analyzer\n\n"
        "Usage: " << program << " <logfile> [options]\n\n"
        "Options:\n"
        "  --top N          show the top N IPs and URLs (default 10)\n"
        "  --status CODE    only count requests with this status code\n"
        "  --ip ADDRESS     only count requests from this IP\n"
        "  --from DATE      only requests on or after DATE (YYYY-MM-DD)\n"
        "  --to DATE        only requests on or before DATE (YYYY-MM-DD)\n"
        "  --threshold N    anomaly threshold: requests per 60s (default 100)\n"
        "  --out PATH       write text report to file\n"
        "  --csv PATH       write CSV report to file\n"
        "  -h, --help       show this help\n";
}

} 