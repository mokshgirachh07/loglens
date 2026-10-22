#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

#include "loglens/Analyzer.h"
#include "loglens/AnomalyDetector.h"
#include "loglens/Options.h"
#include "loglens/Reader.h"
#include "loglens/Report.h"
#include "loglens/Timer.h"

namespace {

bool matches(const loglens::LogEntry& e, const loglens::Options& o) {
    if (o.status && e.status != *o.status) return false;
    if (o.ip && e.ip != *o.ip) return false;
    if (o.fromEpoch && e.epoch < *o.fromEpoch) return false;
    if (o.toEpoch && e.epoch > *o.toEpoch) return false;
    return true;
}

}  // namespace

int main(int argc, char* argv[]) {
    loglens::Options opts;
    std::string error;
    if (!loglens::parseOptions(argc, argv, opts, error)) {
        std::cerr << "Error: " << error << "\n\n";
        loglens::printUsage(argv[0]);
        return 1;
    }
    if (opts.help) {
        loglens::printUsage(argv[0]);
        return 0;
    }

    loglens::Analyzer analyzer;
    loglens::AnomalyDetector detector(opts.threshold, 60);
    std::size_t matched = 0;

    loglens::Timer timer;
    const auto result = loglens::readLog(opts.path, [&](const loglens::LogEntry& e) {
        if (!matches(e, opts)) return;
        ++matched;
        analyzer.add(e);
        detector.add(e);
    });
    const double elapsed = timer.seconds();

    if (!result.opened) {
        std::cerr << "Error: could not open '" << opts.path << "'\n";
        return 1;
    }
    if (result.parsed == 0) {
        std::cerr << "No valid log lines found.\n";
        return 1;
    }

    const loglens::ReportData data{result, analyzer, detector, matched, opts.top, opts.threshold};

    // Always print to the console
    loglens::writeTextReport(std::cout, data);

    // Optionally save to files
    if (!opts.outPath.empty()) {
        std::ofstream file(opts.outPath);
        if (!file) {
            std::cerr << "Error: could not write '" << opts.outPath << "'\n";
            return 1;
        }
        loglens::writeTextReport(file, data);
        std::cout << "\nText report saved to " << opts.outPath << '\n';
    }
    if (!opts.csvPath.empty()) {
        std::ofstream file(opts.csvPath);
        if (!file) {
            std::cerr << "Error: could not write '" << opts.csvPath << "'\n";
            return 1;
        }
        loglens::writeCsvReport(file, data);
        std::cout << "CSV report saved to " << opts.csvPath << '\n';
    }

    // Timing goes to cerr so it never ends up inside saved reports
    std::cerr << std::fixed << std::setprecision(2) << "\nProcessed " << result.totalLines
              << " lines in " << elapsed << " s ("
              << static_cast<double>(result.totalLines) / elapsed / 1e6 << " M lines/s)\n";
    return 0;
}