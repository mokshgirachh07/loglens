#include "loglens/Parser.h"

#include <charconv>
#include <string_view>

#include "loglens/Timestamp.h"

namespace loglens {

namespace {

// Cuts off everything up to the next `delim` and returns it.
// Returns false if the delimiter isn't found.
bool takeUntil(std::string_view& rest, char delim, std::string_view& out) {
    const auto pos = rest.find(delim);
    if (pos == std::string_view::npos) return false;
    out = rest.substr(0, pos);
    rest.remove_prefix(pos + 1);
    return true;
}

template <typename T>
bool toNumber(std::string_view s, T& out) {
    if (s.empty()) return false;
    const auto res = std::from_chars(s.data(), s.data() + s.size(), out);
    return res.ec == std::errc() && res.ptr == s.data() + s.size();
}

}  // namespace

// Line format:
// 10.0.0.14 - - [30/Sep/2026:10:15:32 +0530] "GET /about HTTP/1.1" 200 8123
std::optional<LogEntry> parseLine(const std::string& line) {
    std::string_view rest(line);
    std::string_view ip, ident, user, time, request, method, url, status, bytes;

    if (!takeUntil(rest, ' ', ip)) return std::nullopt;
    if (!takeUntil(rest, ' ', ident)) return std::nullopt;
    if (!takeUntil(rest, ' ', user)) return std::nullopt;

    // [timestamp]
    if (rest.empty() || rest.front() != '[') return std::nullopt;
    rest.remove_prefix(1);
    if (!takeUntil(rest, ']', time)) return std::nullopt;
    if (rest.size() < 2 || rest[0] != ' ' || rest[1] != '"') return std::nullopt;
    rest.remove_prefix(2);

    // "METHOD URL PROTOCOL"
    if (!takeUntil(rest, '"', request)) return std::nullopt;
    if (rest.empty() || rest.front() != ' ') return std::nullopt;
    rest.remove_prefix(1);

    std::string_view req = request;
    if (!takeUntil(req, ' ', method)) return std::nullopt;
    if (!takeUntil(req, ' ', url)) return std::nullopt;

    // status and bytes
    if (!takeUntil(rest, ' ', status)) return std::nullopt;
    bytes = rest;
    while (!bytes.empty() && (bytes.back() == '\r' || bytes.back() == ' ')) bytes.remove_suffix(1);

    if (ip.empty() || method.empty() || url.empty() || status.size() != 3) return std::nullopt;

    int statusCode = 0;
    if (!toNumber(status, statusCode)) return std::nullopt;

    std::size_t byteCount = 0;
    if (bytes != "-" && !toNumber(bytes, byteCount)) return std::nullopt;

    const auto parsedTime = parseTimestamp(std::string(time));
    if (!parsedTime) return std::nullopt;

    LogEntry entry;
    entry.ip = std::string(ip);
    entry.timestamp = std::string(time);
    entry.epoch = parsedTime->epoch;
    entry.hour = parsedTime->hour;
    entry.method = std::string(method);
    entry.url = std::string(url);
    entry.status = statusCode;
    entry.bytes = byteCount;
    return entry;
}

}