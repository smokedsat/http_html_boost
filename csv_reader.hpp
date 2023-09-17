#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

// Struct with data from csv
struct Table {
public:
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> column_names;
    std::vector<std::string> column_types;

    std::string table_filename;

    const std::string EMPTY_ROW = "-";
    char DELIMETER = ';';

    Table(const std::string & filename);
    Table(){ }

public:
    // Reading CSV file
    void 
        readCSV(const std::string& filename);
    bool
        isEmpty();
    void 
        setTableFilename(const std::string & filename);
    void
        setDelimeter(std::ifstream & csv_file);

    // Determinating column types and column names in table from file
    void 
        determineColumnTypesAndNames();

    // Printing table to the console
    void 
        printTable();

    // Clearing table
    void 
        clearTable();

    std::vector<std::string> 
        getColumnNames() const;

    std::vector<std::string> 
        getColumnTypes() const;

    // Getting columns/rows with name
    std::vector<std::string> 
        getColumnsWithName(const std::string& name);

private:
    // Checking if string convertable to double
    bool 
        isNumber(const std::string& s);
};

