#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>


const std::string EMPTY_ROW = "-";
const std::string PATH = "clear_uploaded_file.csv";

// Struct with data from csv
struct Table {
public:
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> column_names;
    std::vector<std::string> column_types;
    char DELIMETER = ';';

        Table(const std::string & filename)
    {
        readCSV(filename);
    }
    Table(){ }

public:
    bool
        isEmpty()
    {
        return table.empty();
    }

    void
        setDelimeter(std::ifstream & csv_file)
    {    
        char character = 'a';
        while(csv_file.get(character))
            {
                if(character == ',')
                {   
                    DELIMETER = ',';
                    break;
                }
                if(character == ';')
                {
                    DELIMETER = ';';
                    break;
                }
            }
    }

    // Determinating column types and column names in table from file
    void 
        determineColumnTypesAndNames() 
    {
        for (const auto& column_data : table) {
            bool is_numeric = true;
            for (const std::string& value : column_data) {
                if (!isNumber(value)) {
                    column_names.push_back(value);
                    is_numeric = false;
                    break;
                }
            }
            column_types.push_back(is_numeric ? "numeric" : "text");
        }
    }

    // Printing table to the console
    void 
        printTable() 
    {
        if (!isEmpty()) {
            size_t numRows = table[0].size();

            // std::cout data with fixed width of row
            for (size_t i = 0; i < numRows; i++) {
                for (size_t j = 0; j < table.size(); j++) {
                    if (i < table[j].size()) {
                        std::cout << std::setw(20) << table[j][i];
                    }
                    else {
                        std::cout << std::setw(20) << " "; // empty row with width 20 if size low 
                    }
                }
                std::cout << std::endl;
            }
        }
        else {
            std::cout << "Error. First you need to read a CSV file." << std::endl;
        }
    }

    // Reading CSV file
    void 
        readCSV(const std::string& filename) 
    {

        std::ifstream csv_file(filename);

        if (!csv_file.is_open()) {
            std::cout << "File is not opened." << std::endl;
            return; // REturn if isnt opened
        }
        
        setDelimeter(csv_file);
        csv_file.seekg(0, std::ios::beg);
        
        std::string header;
        std::getline(csv_file, header);
        
        std::istringstream header_stream(header);
        std::string column_name;

        while (std::getline(header_stream, column_name, DELIMETER)) {
            table.push_back({ column_name }); // Creating vectors with first element = column name
        }

        std::string line;

        while (getline(csv_file, line)) {
            std::istringstream columns_stream(line);
            std::vector<std::string> data_from_columns;
            size_t counter = 0;

            while (std::getline(columns_stream, column_name, DELIMETER)) {
                if (column_name.empty()) {
                    table[counter].push_back(EMPTY_ROW); // Pushing EMPTY_ROW const if in file we find ;; means empty row
                }
                else {
                    table[counter].push_back(column_name); // Pushing data to compatible column
                }
                counter++;
            }
        }
        csv_file.close();

        determineColumnTypesAndNames(); 
    }

    // Clearing table
    void 
        clearTable() 
    {
        table.clear();
        column_names.clear();
        column_types.clear();
    }

    std::vector<std::string> 
        getColumnNames() const 
    {
        if (!column_names.empty())
        {
            return column_names;
        }
        else
        {
            return std::vector<std::string> {};
        }
    }

    std::vector<std::string> 
        getColumnTypes() const 
    {
        if (!column_types.empty())
        {
            return column_types;
        }
        else
        {
            return std::vector<std::string> {};
        }
    }

    // Getting columns/rows with name
    std::vector<std::string> 
        getColumnsWithName(const std::string& name)
    {
        std::vector<std::string> results;
    
    
        for (int i = 0; i < table[0].size(); i++)
        {
            if (!table[i][0].compare(name))
            {
                for (int j = 1; j < table[i].size(); j++)
                {
                    results.push_back(table[i][j]);
                }
                return results;
            }
        }
        return results;
    }

private:

    bool 
        isNumber(const std::string& s) 
    {
        std::istringstream iss(s);
        double number;
        iss >> number;
        // If casting was successful and there is no symbols left, string is number
        return !iss.fail() && iss.eof();
    }
};

