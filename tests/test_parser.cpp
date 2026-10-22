#include <gtest/gtest.h>

#include "loglens/Parser.h"
#include "loglens/Timestamp.h"

using namespace loglens;

TEST(Parser, ParsesValidLine) {
    const auto e = parseLine(
        R"(10.0.0.14 - - [30/Sep/2026:10:15:32 +0530] "GET /about HTTP/1.1" 200 8123)");
    ASSERT_TRUE(e.has_value());
    EXPECT_EQ(e->ip, "10.0.0.14");
    EXPECT_EQ(e->method, "GET");
    EXPECT_EQ(e->url, "/about");
    EXPECT_EQ(e->status, 200);
    EXPECT_EQ(e->bytes, 8123u);
    EXPECT_EQ(e->hour, 10);
}

TEST(Parser, DashBytesBecomesZero) {
    const auto e = parseLine(
        R"(1.2.3.4 - - [30/Sep/2026:10:00:00 +0530] "GET / HTTP/1.1" 200 -)");
    ASSERT_TRUE(e.has_value());
    EXPECT_EQ(e->bytes, 0u);
}

TEST(Parser, UrlWithQueryString) {
    const auto e = parseLine(
        R"(1.2.3.4 - - [30/Sep/2026:10:00:00 +0530] "GET /search?q=cpp HTTP/1.1" 404 12)");
    ASSERT_TRUE(e.has_value());
    EXPECT_EQ(e->url, "/search?q=cpp");
    EXPECT_EQ(e->status, 404);
}

TEST(Parser, RejectsGarbage) {
    EXPECT_FALSE(parseLine("this is a malformed line").has_value());
    EXPECT_FALSE(parseLine("").has_value());
}

TEST(Parser, RejectsNonNumericStatus) {
    EXPECT_FALSE(parseLine(
        R"(1.2.3.4 - - [30/Sep/2026:10:00:00 +0530] "GET /x HTTP/1.1" abc 100)").has_value());
}

TEST(Parser, RejectsBadTimestamp) {
    EXPECT_FALSE(parseLine(
        R"(1.2.3.4 - - [99/Foo/2026:10:00:00 +0530] "GET /x HTTP/1.1" 200 100)").has_value());
}

TEST(Parser, RejectsMissingQuotes) {
    EXPECT_FALSE(parseLine(
        R"(1.2.3.4 - - [30/Sep/2026:10:00:00 +0530] GET /x HTTP/1.1 200 100)").has_value());
}

TEST(Timestamp, EpochStartIsZero) {
    const auto t = parseTimestamp("01/Jan/1970:00:00:00 +0000");
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->epoch, 0);
}

TEST(Timestamp, TimezoneOffsetIsApplied) {
    // 05:30 in +0530 is exactly midnight UTC
    const auto t = parseTimestamp("01/Jan/1970:05:30:00 +0530");
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->epoch, 0);
    EXPECT_EQ(t->hour, 5);  // hour stays as written in the log
}

TEST(Timestamp, OneDayLater) {
    const auto t = parseTimestamp("02/Jan/1970:00:00:00 +0000");
    ASSERT_TRUE(t.has_value());
    EXPECT_EQ(t->epoch, 86400);
}

TEST(Timestamp, LeapDayExists) {
    const auto a = parseTimestamp("28/Feb/2024:00:00:00 +0000");
    const auto b = parseTimestamp("01/Mar/2024:00:00:00 +0000");
    ASSERT_TRUE(a && b);
    EXPECT_EQ(b->epoch - a->epoch, 2 * 86400);  // 2024 is a leap year
}

TEST(Timestamp, RejectsWrongLength) {
    EXPECT_FALSE(parseTimestamp("30/Sep/2026").has_value());
}