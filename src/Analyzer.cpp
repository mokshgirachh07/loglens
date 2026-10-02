#include "loglens/Analyzer.h"

namespace loglens {

void Analyzer::add(const LogEntry& entry) {
    ++statusCounts_[entry.status];
    ++ipCounts_[entry.ip];
}

}