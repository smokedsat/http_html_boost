#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <cstdlib>

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <fstream>
#include <filesystem>
#include "csv_reader.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

const unsigned short port = 7777;

// Delete system webkit data from file. It is 4 lines at begin and 1 line at end. Need to delete because its a trash
void erase_4_1_lines(const std::string& filename);

struct Database : public std::enable_shared_from_this<Database>
{
    std::vector<std::string> results;
    std::vector<std::string> file_names;
    std::vector<Table> tables;

    Table curr_table;

    size_t countOfTables = 0;
    int currentTableId = 0;
};

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    // Constructor to access database in main via shared_ptr
    http_connection(tcp::socket socket, std::shared_ptr<Database> & sptr_database)
    : socket_(std::move(socket))
    {
        this->sptr_database = sptr_database;
    }
    void start()
    {
        read_request();
        check_deadline();
    }

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{ 8192 };
    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;
    net::steady_timer deadline_{ socket_.get_executor(), std::chrono::seconds(60) };
    
    std::shared_ptr<Database> sptr_database;

private:
    void 
        read_request()
    {
        auto self = shared_from_this();
        
        http::async_read(
            socket_,
            buffer_,
            request_,
            [self /*, &shared_tables*/](beast::error_code ec, std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                    self->process_request();
            });
    }

    void 
        write_response()
    {
        auto self = shared_from_this();
    
        response_.content_length(response_.body().size());
    
        http::async_write(
            socket_,
            response_,
            [self](beast::error_code ec, std::size_t)
            {
                
                self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                self->deadline_.cancel();
            });
    }

    void 
        process_request()
    {
        response_.version(request_.version());
        response_.keep_alive(false);
    
        switch (request_.method())
        {
        case http::verb::get:
            if (request_.target().find("/main") == 0)
            {
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            else if (request_.target().find("/showtable") == 0)
            {
                if (request_.target().size() > 11)
                {
                    std::string target = request_.target().substr(11);
                    std::cout << target << std::endl;
                    
                    sptr_database->currentTableId = std::stoi(target); // stoi returns integer from string
                }
    
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                
                response_.prepare_payload();
                create_response();
            }
            else if (request_.target().find("/search") == 0)
            {
                if (sptr_database->tables.empty())
                {
                    // Создаем HTTP-ответ
                    response_.result(http::status::ok);
                    response_.set(http::field::server, "Get request");
                    response_.prepare_payload();
    
                    beast::ostream(request_.body()) << "Tables is empty. Cant start /search.\n ";
                    create_response();
                }
                else
                {
                    if (request_.body().size() > 0)
                    {
                        // Получаем полный путь из URI запроса
                        std::string target_path = request_.target().substr(8);
    
                        // Ищем параметр column_name в URL-адресе
                        size_t param_pos = target_path.find("column_name=");
    
                        if (param_pos != std::string::npos)
                        {
                            // Найден параметр column_name, извлекаем его значение
                            size_t value_start = param_pos + strlen("column_name=");
                            size_t value_end = target_path.find("&", value_start);
                            std::string column_name_value = target_path.substr(value_start, value_end - value_start);
    
                            // Выполняем поиск по значению column_name_value
                            sptr_database->results = sptr_database->tables[sptr_database->currentTableId].getColumnsWithName(column_name_value);
                        }
                    }
                }
                // Создаем HTTP-ответ
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            else
            {
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            break;
        case http::verb::post:
            if (request_.target() == "/upload")
            {
                std::string file_data = beast::buffers_to_string(request_.body().data());
    
                const std::string filename = "uploaded_file_" + std::to_string(sptr_database->countOfTables) + ".csv";
    
                std::ofstream outfile(filename);
                outfile << file_data;
                outfile.close();
    
                erase_4_1_lines(filename);
                std::string newfilename = "clear_" + filename;
                sptr_database->curr_table.readCSV(newfilename);
                sptr_database->tables.emplace_back(sptr_database->curr_table);
                sptr_database->curr_table.clearTable();
                /*if (!sptr_database->tables.empty() && (sptr_database->countOfTables == 0))
                {
                   
                }*/
                sptr_database->countOfTables = sptr_database->tables.size();
    
                sptr_database->file_names.push_back(newfilename);
    
                response_.result(http::status::ok);
                response_.set(http::field::server, "Post request");
                beast::ostream(response_.body()) << "File was successfully uploaded.\n" << "Count of loaded files: " << sptr_database->file_names.size() << "\n";
                response_.prepare_payload();
                create_response();
            }
            else
            {
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "No file data found in the request\r\n";
            }
            break;
        default:
            response_.result(http::status::bad_request);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "Invalid request-method '"
                << std::string(request_.method_string())
                << "'";
            break;
        }
        write_response();
    }

    void 
        create_response()
    {
        if (request_.target().find("/showtable") == 0)
        {
            if (!sptr_database->tables.empty())
            {
                if (request_.target().size() > 11)
                {
                    std::string target = request_.target().substr(11);
                    std::cout << target << std::endl;
    
                    sptr_database->currentTableId = std::stoi(target); // stoi returns integer from string
                    if ((sptr_database->tables.size() < sptr_database->currentTableId))
                    {
                        std::cout << "currentTableId < " << sptr_database->currentTableId << ". It setted to 0. " << std::endl;
                        sptr_database->currentTableId = 0;
                    }
                    beast::ostream(response_.body()) << generateDynamicResponse(sptr_database->tables[sptr_database->currentTableId]);
                }
                else
                {
                    response_.set(http::field::content_type, "text/html");
                    beast::ostream(response_.body()) << "Please make sure you wrote number of table you want to show. Example /showtable/0 \n";
                }
            }
            else
            {
                response_.set(http::field::content_type, "text/html");
                beast::ostream(response_.body()) << "tables is empty. Cant show any data with /showtable. \n";
            }
        }
        else if (request_.target().find("/upload") == 0)
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head>\n"
                << "<title> File Upload </title> \n"
                << "</head>\n"
                << "<body>\n"
                << "<h1> File upload </h1>\n"
                << "<p> <a href=\"/main\">Return to Main Page</a></p>\n"
                << "<form action=\"http://127.0.0.1:"
                << std::to_string(PORT);
            beast::ostream(response_.body())
                << "/upload\" method=\"post\" enctype=\"multipart/form-data\">\n"
                << "<input type=\"file\" name=\"upload-file\">\n"
                << "<input type=\"submit\" value=\"Upload\">\n"
                << "</form>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (request_.target().find("/search") == 0)
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << generateDynamicResponse(sptr_database->results);
        }
        else if (request_.target().find("/main") == 0)
        {           
            response_.result(http::status::ok);
            response_.set(http::field::server, "Get request");
            response_.set(http::field::content_type, "text/html");
                beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Main Page</title></head>\n"
                << "<body>\n"
                << "<h1>Main Page</h1>\n"
                << "<p>Welcome to the main page. Choose an action:</p>\n"
                << "<ul>\n"
                << "<li><a href=\"/upload\">Upload a file</a></li>\n";
            if (sptr_database->countOfTables != 0)
            {
                beast::ostream(response_.body()) << "<li><a href=\"/showtable\">Show table</a></li>\n";
            }
                beast::ostream(response_.body()) 
                << "</ul>\n"
                << "<h2>Search in CSV</h2>\n"
                << "<form action=\"/search\" method=\"get\">\n"
                << "    <label for=\"column_name\">Column Name:</label>\n"
                << "    <input type=\"text\" id=\"column_name\" name=\"column_name\"><br><br>\n"
                << "    <input type=\"submit\" value=\"Search\">\n"
                << "</form>\n";
    
            // Добавьте кнопку "SHOW COLUMN NAMES" для отображения имен столбцов
            if (sptr_database->countOfTables != 0)
            {
                beast::ostream(response_.body())
                    << "<form action=\"/main\" method=\"get\">\n"
                    << "    <input type=\"hidden\" name=\"show_columns\" value=\"true\">\n"
                    << "    <input type=\"submit\" value=\"SHOW COLUMN NAMES\">\n"
                    << "</form>\n";
            }
    
            // Проверьте параметр запроса show_columns и выведите имена столбцов, если он установлен в "true"
    
            if (request_.target().find("show_columns=true") != std::string::npos)
            {
                if (!sptr_database->tables.empty())
                {
                    // Получите список имен столбцов (например, из вашей таблицы)
                    sptr_database->results = sptr_database->tables[sptr_database->currentTableId].getColumnNames();
    
                    beast::ostream(response_.body()) << "<br><strong>Column Names:</strong><br><ul>";
                    for (const std::string& result : sptr_database->results)
                    {
                        beast::ostream(response_.body()) << "<li><strong>" << result << "</strong></li>";
                    }
                    beast::ostream(response_.body()) << "</ul>";
                }
                else
                {
                    beast::ostream(response_.body()) << "<br><strong>Table is empty. Cant find column names.</strong><br><ul>";
                }
            }
    
            beast::ostream(response_.body())
                << "</body>\n"
                << "</html>\n";
        }
    }

    

    void check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](beast::error_code ec)
            {
                if (!ec)
                {
                    self->socket_.close(ec);
                }
            });
    }
    
    // HTML with vector<std::string> result
    std::string 
        generateDynamicResponse(const std::vector<std::string>& data)
    {
        std::stringstream html;
        html << "<!DOCTYPE html>\n";
        html << "<html>\n";
        html << "<head>\n";
        html << "    <title>RESULTS</title>\n";
        html << "</head>\n";
        html << "<body>\n";
    
        if (data.empty()) {
            html << "<h1>table is empty</h1>\n";
        }
        else {
            html << "<h1>RESULTS</h1>\n";
            html << "<table>\n";
            html << "<tr>\n";
    
            // Создаем ячейки таблицы и заполняем их значениями из вектора
            for (const std::string& value : data) {
                html << "<td>" << value << "</td>\n";
            }
    
            html << "</tr>\n";
            html << "</table>\n";
        }
    
        html << "</body>\n";
        html << "</html>\n";
    
        return html.str();
    }

        // HTML with result from Table
    std::string 
        generateDynamicResponse(const Table& table)
    {
        if (!table.table.empty())
        {
            std::stringstream html;
            html << "<!DOCTYPE html>\n";
            html << "<html>\n";
            html << "<head>\n";
            html << "    <title>          ";
            html << table.table_filename;
            html << "           </title>\n";
            html << "</head>\n";
            html << "<body>\n";
            html << "    <h1>                 </h1>\n";
            html << "    <table>\n";
            html << "        <tr>\n";

            
            // Output column names at 1 string. Something wrong
            /*
            for (const std::string& column_name : table.column_names) {
                html << "            <th>" << column_name << "</th>\n";
            }*/

            html << "        </tr>\n";

            for (const auto& rows : table.table) {
                html << "        <tr>\n";
                for (auto i = 0; i < rows.size(); i++) {
                    if (!rows[i].empty() && (i != 0)) 
                    { 
                        html << "            <td>" << rows[i] << "</td>\n"; 
                    }
                    else if (!rows[i].empty() && (i == 0))
                    {
                        html << "            <td><b>" << rows[i] << "</b></td>\n";
                    }
                    else 
                    { 
                        html << "            <td></td>\n"; 
                    }
                }
                html << "        </tr>\n";
            }

            html << "    </table>\n";
            html << "</body>\n";
            html << "</html>\n";

            return html.str();
        }
        else
        {
            std::cout << "Table is empty. " << std::endl;
            return "";
        }
    }
}; // endof class http_connection

void 
    erase_4_1_lines(const std::string& filename)
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

void 
    http_server(tcp::acceptor& acceptor, tcp::socket& socket, std::shared_ptr<Database> & sptr_database)
{
    acceptor.async_accept(socket,
        [&](beast::error_code ec)
        {
            if (!ec)
                std::make_shared<http_connection>(std::move(socket),sptr_database)->start();
            http_server(acceptor, socket, sptr_database);
        });
}

#include <msi.h>
// And just to make things link:
#pragma comment(lib, "msi.lib")

int main(int argc, char* argv[])
{
    Database database_;
    std::shared_ptr<Database> sptr_database_ = std::make_shared<Database>(database_);
    

    if (argc != 3)
    {
        argc = 3;
        argv[0] = (char*)"receiver";
        argv[1] = (char*)"127.0.0.1";
        argv[2] = (char*)"7777";
    }
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
            std::cerr << "  For IPv4, try:\n";
            std::cerr << "    receiver 0.0.0.0 80\n";
            std::cerr << "  For IPv6, try:\n";
            std::cerr << "    receiver 0::0 80\n";
            return EXIT_FAILURE;
        }

        auto const address = net::ip::make_address(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

        net::io_context ioc{ 1 };

        tcp::acceptor acceptor{ ioc, {address, port} };
        tcp::socket socket{ ioc };
        http_server(acceptor, socket, sptr_database_);

        ioc.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return 0;
}
