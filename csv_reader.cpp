#include "csv_reader.h"

Table::Table(const std::string & filename)
{
    readCSV(filename);
}

void 
    Table::setTableFilename(const std::string & filename)
{
    table_filename = filename;
}

bool
        Table::isEmpty()
    {
        return table.empty();
    }

void
    Table::setDelimeter(std::ifstream & csv_file)
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

void 
    Table::determineColumnTypesAndNames() 
{
    bool is_numeric = true;

    for(const auto vec : table)
    {
        if(!vec.empty())
        {
            column_names.push_back(vec[0]);
            if(!isNumber(vec[1])) // vec[0] is name of columns
            {
                is_numeric = false;
            }
            column_types.push_back(is_numeric ? "numeric" : "text");
            is_numeric = true;
        }
    }
}

void 
    Table::printTable() 
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

void 
    Table::readCSV(const std::string& filename) 
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

void 
    Table::clearTable() 
{
    table.clear();
    column_names.clear();
    column_types.clear();
}

std::vector<std::string> 
    Table::getColumnNames() const 
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
    Table::getColumnTypes() const 
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

std::vector<std::string> 
    Table::getColumnsWithName(const std::string& name)
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

bool 
    Table::isNumber(const std::string& s) 
{
    std::istringstream iss(s);
    double number;
    iss >> number;
    // If casting was successful and there is no symbols left, string is number
    return !iss.fail() && iss.eof();
}