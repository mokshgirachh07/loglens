#include <fstream>
#include <iostream>
#include <string>

#include "loglens/Parser.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <logfile>\n";
        return 1;
    }

    const std::string path = argv[1];
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: could not open '" << path << "'\n";
        return 1;
    }

    std::string line;
    std::size_t total = 0, parsed = 0, malformed = 0;

    std::cout << "--- First 3 parsed entries ---\n";
    while (std::getline(file, line)) {
        ++total;
        auto entry = loglens::parseLine(line);
        if (!entry) {
            ++malformed;
            continue;
        }
        if (parsed < 3) {
            std::cout << entry->ip << " | " << entry->method << " " << entry->url
                      << " | " << entry->status << " | " << entry->bytes << " bytes\n";
        }
        ++parsed;
    }

    std::cout << "------------------------------\n";
    std::cout << "Total lines : " << total << '\n';
    std::cout << "Parsed      : " << parsed << '\n';
    std::cout << "Malformed   : " << malformed << '\n';
    return 0;
}