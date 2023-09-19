#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>


// for json
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

const std::string EMPTY_ROW = "-";

using ptree = boost::property_tree::ptree;

// Struct with data from csv
struct Table {
public:
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> column_names;
    std::vector<std::string> column_types;

    std::vector<ptree> json_data_vector;

    char DELIMETER = ';';
    std::string table_filename;

    Table
    (const std::string& filename)
    {
        std::cout << "Table constructor with const std::string &filename" << std::endl;
        readCSV(filename);
    }

    Table() {
        std::cout << "Table default constructor." << std::endl;
    }

public:
    void setTableFilename(const std::string& name)
    {
        table_filename = name;
    }
    bool
        isEmpty()
    {
        std::cout << "isEmpty function." << std::endl;
        return table.empty();
    }

    void
        setDelimeter(std::ifstream& csv_file)
    {
        std::cout << "setDelimeter function." << std::endl;
        char character = 'a';
        while (csv_file.get(character))
        {
            if (character == ',')
            {
                DELIMETER = ',';
                break;
            }
            if (character == ';')
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
        std::cout << "determineColumnTypesAndNames function." << std::endl;
        if (isEmpty())
        {
            std::cout << "table is empty." << std::endl;
            return;
        }
        else
        {
            std::cout << "for(const auto & column_data : table)" << std::endl;
            for (const auto& column_data : table) {
                std::cout << "is_numeric = true;" << std::endl;
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
    }

    // Printing table to the console
    void
        printTable()
    {
        std::cout << "printTable function." << std::endl;
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
        setTableFilename(filename);

        ptree template_ptree;

        std::cout << "ifstream csv_file(filename)" << std::endl;
        std::ifstream csv_file(filename);

        if (!csv_file.is_open()) {
            std::cout << "File is not opened." << std::endl;
            return; // REturn if isnt opened
        }

        std::cout << "SetDelimeter" << std::endl;
        setDelimeter(csv_file);

        std::cout << "csv_file.seekg(0,std::ios::beg)"<< std::endl;
        csv_file.seekg(0, std::ios::beg);

        
        std::string header;

        std::cout << "std::getline(csv_file,header)" << std::endl;
        std::getline(csv_file, header);

        std::cout << "std::istringstream header_stream(header)" << std::endl;
        std::istringstream header_stream(header);

        std::string column_name;

        std::cout << "while(std::getline,header_stream, column_name, DELIMETER)" << std::endl;
        while (std::getline(header_stream, column_name, DELIMETER)) {
            table.push_back({ column_name }); // Creating vectors with first element = column name
            std::cout << "table.size() = " << table.size() << std::endl;

            template_ptree.put(column_name,"");
        }

        std::string line;
        size_t counter = 0;
        ptree current_ptree;

        std::cout << "while(getline(csv_file,line))" << std::endl;
        while (getline(csv_file, line)) {
            if (!line.empty() && line.find_first_not_of('\r') != std::string::npos)
            {
                current_ptree = template_ptree;

                std::cout << "line: " << line.c_str() << std::endl;
                std::istringstream columns_stream(line);
                std::vector<std::string> data_from_columns;
                counter = 0;
                std::cout << "while(std::getline(columns_stream,column_name,DELIMETER))" << std::endl;
                while (std::getline(columns_stream, column_name, DELIMETER)) 
                {
                    std::cout << "countner = " << counter << std::endl;
                    if (column_name.empty()) {

                        current_ptree.put(table[counter][0], EMPTY_ROW);

                        std::cout << "pushed empty_row: " << EMPTY_ROW << std::endl;
                        table[counter].push_back(EMPTY_ROW); // Pushing EMPTY_ROW const if in file we find ;; means empty row
                    }
                    else{
                        current_ptree.put(table[counter][0], column_name);

                        std::cout << "pushed column_name: " << column_name << std::endl;
                        table[counter].push_back(column_name); // Pushing data to compatible column
                    }

                    std::cout << "counter++" << std::endl;
                    if(counter != table.size() - 1)
                    {
                        counter++;
                    }
                }
                json_data_vector.push_back(current_ptree);
                current_ptree.clear();
            }
            else
            {
                current_ptree.clear();
                std::cout << "skip line" << std::endl;
            }
        }
        std::cout << "csv_file.close();" << std::endl;
        csv_file.close();
        std::cout << "DetermineColumnTypesAndNames();" << std::endl;
        determineColumnTypesAndNames();
        printJson();
    }


    void printJson()
    {
        std::string fname = table_filename + ".json";
        std::ofstream json_out(fname,std::ios::trunc);

        if (!json_out.is_open()) 
        {
            std::cout << "Failed to open " << fname << std::endl;
            return;
        }

        for(auto& i : json_data_vector)
        {
            boost::property_tree::write_json(json_out,i);
            std::cout << i.data() << std::endl;
        }
        json_out.close();
    }

    // Clearing table
    void
        clearTable()
    {
        std::cout << "clearTable function." << std::endl;
        if(!table.empty())
        {
            table.clear();
        }
        if(!column_names.empty())
        {
            column_names.clear();
        }
        
        if(!column_types.empty())
        {
            column_types.clear();
        }
        
        if(!table_filename.empty())
        {
            table_filename.clear();
        }
        
    }

    std::vector<std::string>
        getColumnNames() const
    {
        std::cout << "getColumnNames function." << std::endl;
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
        std::cout << "getColumnTypes function." << std::endl;
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
        std::cout << "getColumnsWithName function." << std::endl;
        std::vector<std::string> results;

        if (!isEmpty())
        {
            std::cout << "Table is not empty." << std::endl;
            for (int i = 0; i < table.size(); i++)
            {
                if(!table[i][0].empty())
                {
                    if (!table[i][0].compare(name))
                    {
                        std::cout << "table[i][0] == " << name << std::endl;
                        for (int j = 1; j < table[i].size(); j++)
                        {
                            std::cout << "results.push_back( " << table[i][j] << ")" << std::endl;
                            results.push_back(table[i][j]);
                        }
                        return results;
                    }
                }
            }
        }
        return results;
    }

private:

    bool
        isNumber(const std::string& s)
    {
        std::cout << "isNumber function." << std::endl;
        std::istringstream iss(s);
        double number;
        iss >> number;
        // If casting was successful and there is no symbols left, string is number
        return !iss.fail() && iss.eof();
    }
};

