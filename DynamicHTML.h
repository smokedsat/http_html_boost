#pragma once

#include "csv_reader.h"
#include <sstream>
#include <vector>
#include <string>

// HTML with vector<std::string> result
std::string 
    generateDynamicResponse(const std::vector<std::string>& data, const std::string & ip_address, const std::string & port);

// HTML with result from Table
std::string 
    generateDynamicResponse(const Table& table, const std::string& ip_address, const std::string& port);
