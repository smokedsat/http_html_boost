#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>

const char DELIMETER = ';';
const std::string EMPTY_ROW = "-";
const std::string PATH = "clear_uploaded_file.csv";

// Структура для хранения данных таблицы и её метаданных
struct Table {
public:
    std::vector<std::vector<std::string>> table;
    std::vector<std::string> column_names;
    std::vector<std::string> column_types;
    bool isTableRead = false;

    Table(const std::string & filename)
    {
        readCSV(filename);
        determineColumnTypes(); // Определение типов данных
    }
    Table()
    {

    }

public:
    // Определение типов данных в колонках
    void determineColumnTypes() {
        for (const auto& column_data : table) {
            bool is_numeric = true;
            for (const std::string& value : column_data) {
                if (!isNumber(value)) {
                    is_numeric = false;
                    break;
                }
            }
            column_types.push_back(is_numeric ? "numeric" : "text");
        }
    }

    // Вывод таблицы в консоль
    void printTable() {
        if (isTableRead) {
            size_t numRows = table[0].size();

            // Выводим значения с фиксированной шириной столбца
            for (size_t i = 0; i < numRows; i++) {
                for (size_t j = 0; j < table.size(); j++) {
                    if (i < table[j].size()) {
                        std::cout << std::setw(20) << table[j][i];
                    }
                    else {
                        std::cout << std::setw(20) << " "; // Заполняем пустыми значениями, если колонка короче
                    }
                }
                std::cout << std::endl;
            }
        }
        else {
            std::cout << "Error. First you need to read a CSV file." << std::endl;
        }
    }

    // Чтение CSV файла
    void readCSV(const std::string& filename) {
        char delimiter = ';';

        std::ifstream csv_file(filename);

        if (!csv_file.is_open()) {
            std::cout << "File is not opened." << std::endl;
            return; // Возвращаемся в случае ошибки открытия файла
        }

        std::string header;
        std::getline(csv_file, header);
        std::istringstream header_stream(header);
        std::string column_name;

        while (std::getline(header_stream, column_name, delimiter)) {
            table.push_back({ column_name }); // Создаём вектор с одним элементом (названием колонки)
        }

        std::string line;
        size_t size = table.size();

        while (getline(csv_file, line)) {
            std::istringstream columns_stream(line);
            std::vector<std::string> data_from_columns;
            size_t counter = 0;

            while (std::getline(columns_stream, column_name, delimiter) && (counter != size)) {
                if (column_name.empty()) {
                    table[counter].push_back(EMPTY_ROW); // Добавляем константу - если значение в таблице отсутствует
                }
                else {
                    table[counter].push_back(column_name); // Добавляем значение в соответствующую колонку
                }
                counter++;
            }
        }

        isTableRead = true;
        csv_file.close();
    }

    // Очистка таблицы
    void clearTable() {
        table.clear();
        column_names.clear();
        column_types.clear();
        isTableRead = false;
    }

    // Получение метаданных о структуре данных
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
        // Если преобразование прошло успешно и не осталось непрочитанных символов,
        // то считаем строку числом.
        return !iss.fail() && iss.eof();
    }
};

int main__() {
    Table current(PATH);
    current.readCSV(PATH);
    current.determineColumnTypes(); // Определение типов данных
    current.printTable();
    //current.clearTable(); // Очистка данных после использования
    return 0;
}