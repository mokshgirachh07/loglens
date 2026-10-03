#include "loglens/Timestamp.h"

#include <cstddef>

namespace loglens {

namespace {

// Reads `len` digits starting at `pos`. Returns false if any is not a digit.
bool readDigits(const std::string& s, std::size_t pos, std::size_t len, int& out) {
    int value = 0;
    for (std::size_t i = pos; i < pos + len; ++i) {
        if (s[i] < '0' || s[i] > '9') return false;
        value = value * 10 + (s[i] - '0');
    }
    out = value;
    return true;
}

// Days since 1970-01-01 for a calendar date (Howard Hinnant's algorithm).
std::int64_t daysFromCivil(int y, unsigned m, unsigned d) {
    y -= (m <= 2);
    const int era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return static_cast<std::int64_t>(era) * 146097 + static_cast<std::int64_t>(doe) - 719468;
}

}  // namespace

std::optional<ParsedTime> parseTimestamp(const std::string& s) {
    // Fixed layout: dd/Mon/yyyy:HH:MM:SS +ZZZZ  (26 characters)
    if (s.size() != 26) return std::nullopt;
    if (s[2] != '/' || s[6] != '/' || s[11] != ':' || s[14] != ':' ||
        s[17] != ':' || s[20] != ' ' || (s[21] != '+' && s[21] != '-')) {
        return std::nullopt;
    }

    int day, year, hour, minute, second, tzHour, tzMin;
    if (!readDigits(s, 0, 2, day) || !readDigits(s, 7, 4, year) ||
        !readDigits(s, 12, 2, hour) || !readDigits(s, 15, 2, minute) ||
        !readDigits(s, 18, 2, second) || !readDigits(s, 22, 2, tzHour) ||
        !readDigits(s, 24, 2, tzMin)) {
        return std::nullopt;
    }

    static const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                   "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    int month = 0;
    for (int i = 0; i < 12; ++i) {
        if (s.compare(3, 3, months[i]) == 0) {
            month = i + 1;
            break;
        }
    }

    if (month == 0 || day < 1 || day > 31 || hour > 23 || minute > 59 || second > 59) {
        return std::nullopt;
    }

    const std::int64_t days = daysFromCivil(year, static_cast<unsigned>(month),
                                            static_cast<unsigned>(day));
    const std::int64_t offset = (tzHour * 3600 + tzMin * 60) * (s[21] == '+' ? 1 : -1);

    ParsedTime t;
    t.epoch = days * 86400 + hour * 3600 + minute * 60 + second - offset;
    t.hour = hour;
    return t;
}

}