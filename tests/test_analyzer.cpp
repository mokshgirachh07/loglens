#include <gtest/gtest.h>

#include "loglens/Analyzer.h"

using namespace loglens;

namespace {

LogEntry makeEntry(const std::string& ip, const std::string& url, int status,
                   std::size_t bytes = 100, int hour = 10) {
    LogEntry e;
    e.ip = ip;
    e.url = url;
    e.status = status;
    e.bytes = bytes;
    e.hour = hour;
    return e;
}

}  // namespace

TEST(Analyzer, CountsStatusCodes) {
    Analyzer a;
    a.add(makeEntry("1.1.1.1", "/", 200));
    a.add(makeEntry("1.1.1.1", "/", 200));
    a.add(makeEntry("1.1.1.1", "/", 404));

    EXPECT_EQ(a.statusCounts().at(200), 2u);
    EXPECT_EQ(a.statusCounts().at(404), 1u);
    EXPECT_EQ(a.statusCounts().count(500), 0u);
}

TEST(Analyzer, SumsBytes) {
    Analyzer a;
    a.add(makeEntry("1.1.1.1", "/", 200, 1000));
    a.add(makeEntry("2.2.2.2", "/", 200, 500));
    EXPECT_EQ(a.totalBytes(), 1500u);
}

TEST(Analyzer, TopIpsSortedBiggestFirst) {
    Analyzer a;
    for (int i = 0; i < 5; ++i) a.add(makeEntry("busy", "/", 200));
    for (int i = 0; i < 2; ++i) a.add(makeEntry("quiet", "/", 200));
    a.add(makeEntry("rare", "/", 200));

    const auto top = a.topIps(2);
    ASSERT_EQ(top.size(), 2u);
    EXPECT_EQ(top[0].first, "busy");
    EXPECT_EQ(top[0].second, 5u);
    EXPECT_EQ(top[1].first, "quiet");
}

TEST(Analyzer, TopNLargerThanDataReturnsEverything) {
    Analyzer a;
    a.add(makeEntry("a", "/", 200));
    a.add(makeEntry("b", "/", 200));
    EXPECT_EQ(a.topIps(10).size(), 2u);
}

TEST(Analyzer, TiesBrokenAlphabetically) {
    Analyzer a;
    a.add(makeEntry("zeta", "/", 200));
    a.add(makeEntry("alpha", "/", 200));

    const auto top = a.topIps(2);
    ASSERT_EQ(top.size(), 2u);
    EXPECT_EQ(top[0].first, "alpha");
    EXPECT_EQ(top[1].first, "zeta");
}

TEST(Analyzer, TopUrls) {
    Analyzer a;
    a.add(makeEntry("1.1.1.1", "/login", 200));
    a.add(makeEntry("2.2.2.2", "/login", 200));
    a.add(makeEntry("3.3.3.3", "/about", 200));

    const auto top = a.topUrls(1);
    ASSERT_EQ(top.size(), 1u);
    EXPECT_EQ(top[0].first, "/login");
    EXPECT_EQ(top[0].second, 2u);
}

TEST(Analyzer, HourlyBuckets) {
    Analyzer a;
    a.add(makeEntry("1.1.1.1", "/", 200, 1, 3));
    a.add(makeEntry("1.1.1.1", "/", 200, 1, 3));
    a.add(makeEntry("1.1.1.1", "/", 200, 1, 23));

    EXPECT_EQ(a.hourly()[3], 2u);
    EXPECT_EQ(a.hourly()[23], 1u);
    EXPECT_EQ(a.hourly()[0], 0u);
}

TEST(Analyzer, EmptyAnalyzerIsSafe) {
    Analyzer a;
    EXPECT_TRUE(a.topIps(5).empty());
    EXPECT_EQ(a.totalBytes(), 0u);
}