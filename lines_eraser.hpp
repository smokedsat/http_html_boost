#pragma once

#include <iostream>
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>

// Delete system webkit data from file. It is 4 lines at begin and 1 line at end. Need to delete because its a trash
void lines_eraser(const std::string& filename);