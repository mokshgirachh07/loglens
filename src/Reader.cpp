#include "loglens/Reader.h"

#include <fstream>

#include "loglens/Parser.h"

namespace loglens {

ReadResult readLog(const std::string& path, const EntryCallback& onEntry) {
    ReadResult result;

    std::ifstream file(path);
    if (!file.is_open()) {
        return result;  // opened stays false
    }
    result.opened = true;

    std::string line;
    while (std::getline(file, line)) {
        ++result.totalLines;
        auto entry = parseLine(line);
        if (!entry) {
            ++result.malformed;
            continue;
        }
        ++result.parsed;
        onEntry(*entry);
    }
    return result;
}

}