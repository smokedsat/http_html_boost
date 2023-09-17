#pragma once

#include <memory>
#include <vector>
#include <string>
#include "csv_reader.h"

struct Database : public std::enable_shared_from_this<Database>
{
    std::vector<std::string> results;
    std::vector<Table> tables;

    Table curr_table;

    size_t countOfTables = 0;
    int currentTableId = 0;
};