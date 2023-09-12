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
    bool isTableRead = false;
    char DELIMETER = ';';

    Table(const std::string & filename)
    {
        readCSV(filename);
    }
    Table()
    {

    }

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
    void determineColumnTypes() {
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

    // Âûâîä òàáëèöû â êîíñîëü
    void printTable() {
        if (isTableRead) {
            size_t numRows = table[0].size();

            // Âûâîäèì çíà÷åíèÿ ñ ôèêñèðîâàííîé øèðèíîé ñòîëáöà
            for (size_t i = 0; i < numRows; i++) {
                for (size_t j = 0; j < table.size(); j++) {
                    if (i < table[j].size()) {
                        std::cout << std::setw(20) << table[j][i];
                    }
                    else {
                        std::cout << std::setw(20) << " "; // Çàïîëíÿåì ïóñòûìè çíà÷åíèÿìè, åñëè êîëîíêà êîðî÷å
                    }
                }
                std::cout << std::endl;
            }
        }
        else {
            std::cout << "Error. First you need to read a CSV file." << std::endl;
        }
    }

    // ×òåíèå CSV ôàéëà
    void readCSV(const std::string& filename) {
        char delimiter = ';';

        std::ifstream csv_file(filename);

        if (!csv_file.is_open()) {
            std::cout << "File is not opened." << std::endl;
            return; // Âîçâðàùàåìñÿ â ñëó÷àå îøèáêè îòêðûòèÿ ôàéëà
        }

        std::string header;
        std::getline(csv_file, header);
        std::istringstream header_stream(header);
        std::string column_name;

        while (std::getline(header_stream, column_name, delimiter)) {
            table.push_back({ column_name }); // Ñîçäà¸ì âåêòîð ñ îäíèì ýëåìåíòîì (íàçâàíèåì êîëîíêè)
        }

        std::string line;
        size_t size = table.size();

        while (getline(csv_file, line)) {
            std::istringstream columns_stream(line);
            std::vector<std::string> data_from_columns;
            size_t counter = 0;

            while (std::getline(columns_stream, column_name, delimiter) && (counter != size)) {
                if (column_name.empty()) {
                    table[counter].push_back(EMPTY_ROW); // Äîáàâëÿåì êîíñòàíòó - åñëè çíà÷åíèå â òàáëèöå îòñóòñòâóåò
                }
                else {
                    table[counter].push_back(column_name); // Äîáàâëÿåì çíà÷åíèå â ñîîòâåòñòâóþùóþ êîëîíêó
                }
                counter++;
            }
        }

        isTableRead = true;
        csv_file.close();
    }

    // Î÷èñòêà òàáëèöû
    void clearTable() {
        table.clear();
        column_names.clear();
        column_types.clear();
        isTableRead = false;
    }

    // Ïîëó÷åíèå ìåòàäàííûõ î ñòðóêòóðå äàííûõ
    std::vector<std::string> getColumnNames() const {
        return column_names;
    }

    std::vector<std::string> getColumnTypes() const {
        return column_types;
    }

private:
    bool isNumber(const std::string& s) {
        std::istringstream iss(s);
        double number;
        iss >> number;
        // Åñëè ïðåîáðàçîâàíèå ïðîøëî óñïåøíî è íå îñòàëîñü íåïðî÷èòàííûõ ñèìâîëîâ,
        // òî ñ÷èòàåì ñòðîêó ÷èñëîì.
        return !iss.fail() && iss.eof();
    }
};

int main__() {
    Table current(PATH);
    current.readCSV(PATH);
    current.determineColumnTypes(); // Îïðåäåëåíèå òèïîâ äàííûõ
    current.printTable();
    //current.clearTable(); // Î÷èñòêà äàííûõ ïîñëå èñïîëüçîâàíèÿ
    return 0;
}
