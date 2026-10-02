#include <fstream>
#include <iostream>
#include <string>

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
    std::size_t total = 0;

    std::cout << "--- First 5 lines ---\n";
    while (std::getline(file, line)) {
        if (total < 5) {
            std::cout << line << '\n';
        }
        ++total;
    }

    std::cout << "---------------------\n";
    std::cout << "Total lines: " << total << '\n';
    return 0;
}