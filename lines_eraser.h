#pragma once

#include <string>
#include <filesystem>
#include <fstream>
#include <vector>

// Delete system webkit data from file. It is 4 lines at begin and 1 line at end. Need to delete because its a trash
void lines_eraser(const std::string& filename)
{
    std::ifstream inputFile(filename);
    std::ofstream outputFile("clear_" + filename);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Failed to open file to erase 4_1_lines." << std::endl;
        return;
    }

    std::vector<std::string> lines;
    std::string line;

    while (std::getline(inputFile, line)) {
        lines.push_back(line);
    }

    if (lines.size() < 4) {
        std::cerr << "File does not contain enough lines." << std::endl;
        return;
    }

    for (auto i = 0; i <= 3; i++)
    {
        lines.erase(lines.begin());
    }

    lines.pop_back();

    for (const std::string& outputLine : lines) {
        outputFile << outputLine << '\n';
    }

    inputFile.close();
    outputFile.close();

    std::filesystem::remove(filename);
}
