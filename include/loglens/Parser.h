#pragma once

#include <optional>
#include <string>

#include "loglens/LogEntry.h"

namespace loglens {

// Returns a LogEntry if the line is valid, std::nullopt otherwise.
std::optional<LogEntry> parseLine(const std::string& line);

} 