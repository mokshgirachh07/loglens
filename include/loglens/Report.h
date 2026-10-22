#pragma once

#include <cstddef>
#include <ostream>

#include "loglens/Analyzer.h"
#include "loglens/AnomalyDetector.h"
#include "loglens/Reader.h"

namespace loglens {

struct ReportData {
    const ReadResult& read;
    const Analyzer& analyzer;
    const AnomalyDetector& detector;
    std::size_t matched;
    std::size_t top;
    std::size_t threshold;
};

// Human-readable report (console or .txt file)
void writeTextReport(std::ostream& out, const ReportData& data);

// Machine-readable report: section,key,value rows
void writeCsvReport(std::ostream& out, const ReportData& data);

}