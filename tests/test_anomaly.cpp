#include <gtest/gtest.h>

#include "loglens/AnomalyDetector.h"

using namespace loglens;

namespace {

LogEntry at(const std::string& ip, std::int64_t epoch) {
    LogEntry e;
    e.ip = ip;
    e.epoch = epoch;
    return e;
}

}  // namespace

TEST(Anomaly, FlagsBurstAboveThreshold) {
    AnomalyDetector d(100, 60);
    for (int i = 0; i < 150; ++i) d.add(at("attacker", 1000));

    const auto alerts = d.alerts();
    ASSERT_EQ(alerts.size(), 1u);
    EXPECT_EQ(alerts[0].ip, "attacker");
    EXPECT_EQ(alerts[0].peakRequests, 150u);
}

TEST(Anomaly, ExactlyAtThresholdIsNotFlagged) {
    AnomalyDetector d(100, 60);
    for (int i = 0; i < 100; ++i) d.add(at("edge", 1000));
    EXPECT_TRUE(d.alerts().empty());
}

TEST(Anomaly, OneAboveThresholdIsFlagged) {
    AnomalyDetector d(100, 60);
    for (int i = 0; i < 101; ++i) d.add(at("edge", 1000));
    EXPECT_EQ(d.alerts().size(), 1u);
}

TEST(Anomaly, SlowTrafficIsNotFlagged) {
    AnomalyDetector d(100, 60);
    // 1000 requests, one every 10 seconds: at most 6 per window
    for (int i = 0; i < 1000; ++i) d.add(at("steady", i * 10));
    EXPECT_TRUE(d.alerts().empty());
}

TEST(Anomaly, OldRequestsLeaveTheWindow) {
    AnomalyDetector d(100, 60);
    // 90 requests at t=0, then 90 more at t=120: never more than 90 in any window
    for (int i = 0; i < 90; ++i) d.add(at("wave", 0));
    for (int i = 0; i < 90; ++i) d.add(at("wave", 120));
    EXPECT_TRUE(d.alerts().empty());
}

TEST(Anomaly, IpsAreTrackedSeparately) {
    AnomalyDetector d(100, 60);
    // 60 requests each: 120 total, but neither IP crosses 100 alone
    for (int i = 0; i < 60; ++i) {
        d.add(at("a", 500));
        d.add(at("b", 500));
    }
    EXPECT_TRUE(d.alerts().empty());
}

TEST(Anomaly, AlertsSortedByPeak) {
    AnomalyDetector d(10, 60);
    for (int i = 0; i < 20; ++i) d.add(at("small", 100));
    for (int i = 0; i < 50; ++i) d.add(at("big", 100));

    const auto alerts = d.alerts();
    ASSERT_EQ(alerts.size(), 2u);
    EXPECT_EQ(alerts[0].ip, "big");
    EXPECT_EQ(alerts[1].ip, "small");
}

TEST(Anomaly, EachIpReportedOnce) {
    AnomalyDetector d(10, 60);
    for (int i = 0; i < 500; ++i) d.add(at("spammer", 100));
    EXPECT_EQ(d.alerts().size(), 1u);
}