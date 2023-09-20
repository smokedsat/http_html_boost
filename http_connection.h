#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <cstdlib>
#include <memory>
#include <iostream>
#include <vector>
#include <fstream>

#include "csv_reader.h"
#include "lines_eraser.h"
#include "DynamicHTML.h"
#include "database.h"

//#define DEBUG_INFORMATION
//#define LOG_TO_FILE  // not supported yet

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

const unsigned int PORT = 7777;
const std::string IP_ADDRESS = "127.0.0.1";

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket, std::shared_ptr<Database>& sptr_database)
        : socket_(std::move(socket))
    {
            #ifdef DEBUG_INFORMATION
                std::cout << "http_connection constructor with tcp::socket, std::shared_ptr<Database>." << std::endl;
            #endif /* DEBUG_INFORMATION */
        this->sptr_database = sptr_database;
    }

    void start()
    {
            #ifdef DEBUG_INFORMATION
                std::cout << "start function." << std::endl;
            #endif /* DEBUG_INFORMATION */
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
            #ifdef DEBUG_INFORMATION
                std::cout << "read_request function." << std::endl;
            #endif /* DEBUG_INFORMATION */
        auto self = shared_from_this();

        http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec, std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                    self->process_request();
            });
    }

    void 
        write_response()
    {
            #ifdef DEBUG_INFORMATION
                std::cout << "wrte_response function." << std::endl;
            #endif /* DEBUG_INFORMATION */
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

    void check_deadline()
    {
            #ifdef DEBUG_INFORMATION
                std::cout << "check_deadline function." << std::endl;
            #endif /* DEBUG_INFORMATION */
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

    void process_request()
    {
            #ifdef DEBUG_INFORMATION
                std::cout << "process_request function." << std::endl;
            #endif /* DEBUG_INFORMATION */
        response_.version(request_.version());
        response_.keep_alive(false);

        switch (request_.method())
        {
        case http::verb::get:
            if (request_.target().find("/main") == 0)
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::get /main" << std::endl;
                    #endif /* DEBUG_INFORMATION */
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            else if (request_.target().find("/set") == 0)
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::get /set" << std::endl;
                    #endif /* DEBUG_INFORMATION */
                if (sptr_database->tables.empty())
                {
                    // ������� HTTP-�����
                    response_.result(http::status::ok);
                    response_.set(http::field::server, "Get request");
                    response_.prepare_payload();

                    beast::ostream(response_.body()) << "Tables is empty. Cant set table to any number with /set.\n ";
                    create_response();
                }
                else
                {
                    if (request_.target().size() > 5)
                    {
                        // �������� ������ ���� �� URI �������
                        #ifdef __linux__
                        std::string target = request_.target().substr(5).to_string();
                        #elif defined(_WIN32) || defined(_WIN64)
                        std::string target = request_.target().substr(5);
                        #endif

                        int number = std::stoi(target);

                        if (sptr_database->countOfTables >= number)
                        {
                            sptr_database->currentTableId = number;
                            beast::ostream(response_.body()) << "Current table was changed to " << std::to_string(number) << "\n";
                        }
                        else
                        {
                            beast::ostream(response_.body())
                                << "<p>Count of tables is lower than you trying to change to.</p>\n "
                                << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
                        }
                    }
                }
                // ������� HTTP-�����
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            else if (request_.target().find("/search") == 0)
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::get /search" << std::endl;
                    #endif /* DEBUG_INFORMATION */
                if (sptr_database->tables.empty())
                {
                    // ������� HTTP-�����
                    response_.result(http::status::ok);
                    response_.set(http::field::server, "Get request");
                    response_.prepare_payload();

                    beast::ostream(response_.body()) << "Tables is empty. Cant start /search.\n ";
                    create_response();
                }
                else
                {
                    if (request_.target().size() > 8)
                    {
                        // �������� ������ ���� �� URI �������
                        #ifdef __linux__
                        std::string target_path = request_.target().substr(8).to_string();
                                #ifdef DEBUG_INFORMATION
                                    std::cout << "target: " << request_.target().to_string()<< std::endl;
                                    std::cout << "target_path: " << target_path << std::endl;
                                #endif // DEBUG_INFORMATION
                        #elif defined(_WIN32) || defined(_WIN64)
                        std::string target_path = request_.target().substr(8);
                                #ifdef DEBUG_INFORMATION
                                    std::cout << "target: " << request_.target()<< std::endl;
                                    std::cout << "target_path: " << target_path << std::endl;
                                #endif // DEBUG_INFORMATION
                        #endif // __linux__ _WIN32 _WIN64

                        size_t param_pos = target_path.find("column_name=");

                        if (param_pos != std::string::npos)
                        {
                            size_t value_start = target_path.find("column_name=") + strlen("column_name=");
                            size_t value_end = target_path.find("&", value_start);
                            if (value_end == std::string::npos) {
                                // if symbol & not finud then is last param 
                                value_end = target_path.length();
                            }
                            std::string column_name_value = target_path.substr(value_start, value_end - value_start);
                            
                                #ifdef DEBUG_INFORMATION
                                    std::cout << "column_name_value = " << column_name_value << std::endl;
                                #endif /* DEBUG_INFORMATION*/
    
                            if(!column_name_value.empty())
                            {
                                sptr_database->results = sptr_database->tables[sptr_database->currentTableId].getColumnsWithName(column_name_value);
                            }
                            else
                            {
                                if(!sptr_database->results.empty())
                                {
                                    sptr_database->results.clear();
                                }
                                sptr_database->results.push_back("Empty query.");
                            }
                        }
                    }
                }
                // ������� HTTP-�����
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            else if (request_.target().find("/showtable") == 0)
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::get /showtable" << std::endl;
                    #endif /* DEBUG_INFORMATION */
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");

                response_.prepare_payload();
                create_response();
            }
            else if(request_.target().find("/clean") == 0)
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::get /clean." << std::endl;
                    #endif /* DEBUG_INFORMATION */
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                sptr_database->clean();

                response_.prepare_payload();
                create_response();
            }
            else if(request_.target().find("/favicon.ico") == 0)
            {
                // nothing. 
            }
            else
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::get empty." << std::endl;
                    #endif /* DEBUG_INFORMATION */
                response_.result(http::status::method_not_allowed);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            break;
        case http::verb::post:
            if (request_.target() == "/upload")
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::post /upload." << std::endl;
                        std::cout << "getting file_data from buffer of request body." << std::endl;
                    #endif /* DEBUG_INFORMATION */
                std::string file_data = beast::buffers_to_string(request_.body().data());
                const std::string filename = "uploaded_file_" + std::to_string(sptr_database->countOfTables) + ".csv";

                std::ofstream outfile(filename,std::ofstream::trunc);
                outfile << file_data;
                outfile.close();

                lines_eraser(filename);
                std::string newfilename = "clear_" + filename;
                    #ifdef DEBUG_INFORMATION
                        std::cout << "curr_table.readCSV" << std::endl;
                    #endif /* DEBUG_INFORMATION */
                sptr_database->curr_table.readCSV(newfilename);
                
// #ifdef DEBUG_INFORMATION
// std::cout << "curr_table.setTableFilename: " << filename << std::endl;
// #endif /* DEBUG_INFORMATION */
                
//                 sptr_database->curr_table.setTableFilename(filename);
                    #ifdef DEBUG_INFORMATION
                        std::cout << "tables.emplace_back(sptr_database->curr_table)" << std::endl;
                    #endif /* DEBUG_INFORMATION */
                
                sptr_database->tables.emplace_back(sptr_database->curr_table);

                if(!sptr_database->tables.empty())
                {
                        #ifdef DEBUG_INFORMATION
                            std::cout << "countOfTables = sptr_database->tables.size()";
                        #endif /* DEBUG_INFORMATION */
                    
                    sptr_database->countOfTables = sptr_database->tables.size();
                        #ifdef DEBUG_INFORMATION
                            std::cout << "curr_table.clearTable()" << std::endl;
                        #endif /* DEBUG_INFORMATION */
                    
                    sptr_database->curr_table.clearTable();
                }
                
                response_.result(http::status::ok);
                response_.set(http::field::server, "Post request");
                beast::ostream(response_.body()) << "File was successfully uploaded.\n" << "Count of loaded files: " << sptr_database->tables.size() << "\n";
                response_.prepare_payload();
                create_response();
            }
            else
            {
                    #ifdef DEBUG_INFORMATION
                        std::cout << "case http::verb::post bad_request." << std::endl;
                    #endif /* DEBUG_INFORMATION */
                response_.result(http::status::bad_request);
                response_.set(http::field::content_type, "text/plain");
                beast::ostream(response_.body()) << "No file data found in the request\r\n";
            }
            break;
        default:
                #ifdef DEBUG_INFORMATION
                    std::cout << "case default bad_request." << std::endl;
                #endif /* DEBUG_INFORMATION */
            response_.result(http::status::bad_request);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body())
                << "Invalid request-method '"
                << std::string(request_.method_string())
                << "'";
            break;
        }
            #ifdef DEBUG_INFORMATION
                std::cout << "write_response()." << std::endl;
            #endif /* DEBUG_INFORMATION */
        
        write_response();
    }

    void create_response()
    {
            #ifdef DEBUG_INFORMATION
                #ifdef __linux__
                    std::cout << "create_response function." << std::endl;
                    std::cout << "request_.target() = " << request_.target().to_string() <<std::endl;
                #elif defined(_WIN32) || defined(_WIN64)
                    std::cout << "create_response function." << std::endl;
                    std::cout << "request_.target() = " << request_.target() <<std::endl;
                #endif
            #endif /* DEBUG_INFORMATION */

        if (request_.target().find("/main") == 0)
        {
            response_.body().clear();
            response_.result(http::status::ok);
            response_.set(http::field::server, "Get request");
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<!DOCTYPE html>\n" // this is new remove
                << "<html>\n"
                << "<head><title>Main Page</title></head>\n"
                << "<body>\n"
                << "<h1>Main Page</h1>\n"
                << "<p>Welcome to the main page. Choose an action:</p>\n"
                << "<ul>\n"
                << "<li><a href=\"/upload\">Upload a file</a></li>\n"
                << "<li><a href=\"/clean\">Clean uploaded files.</a></li>\n";

            if (!sptr_database->tables.empty())
            {
                for (size_t count = 0; count < sptr_database->countOfTables; count++)
                {
                    beast::ostream(response_.body()) << "<li>Set table to <a href=\"/set/" + std::to_string(count);
                    beast::ostream(response_.body()) << "\"> ";
                    beast::ostream(response_.body()) << sptr_database->tables[count].table_filename;
                    beast::ostream(response_.body()) << " </a></li>\n";
                }
                beast::ostream(response_.body()) << "<br>\n";
            }
            if (!sptr_database->tables.empty())
            {
                // Button showtable
                beast::ostream(response_.body())
                    << "<form action=\"http://127.0.0.1:" << std::to_string(PORT)
                    << "/showtable/\" method=\"get\">\n" // ���������� ����� GET
                    << "<li><b>Enter Table Number:   </b> <input type=\"number\" name=\"tableNumber\">\n"
                    << "    <input type=\"submit\" value=\"Show Table\"></li>\n"
                    << "</form>\n";

                // Button search 
                beast::ostream(response_.body())
                    << "<b><p>Search in table</p></b>"
                    << "<form action=\"/search\" method=\"get\">\n"
                    << "    <label for=\"column_name\">Column Name:</label>\n"
                    << "    <input type=\"text\" id=\"column_name\" name=\"column_name\">"
                    << "    <input type=\"submit\" value=\"Search\">\n"
                    << "</form>\n";

                // Getting column_names from table
                sptr_database->results = sptr_database->tables[sptr_database->currentTableId].getColumnNames();

                beast::ostream(response_.body())
                    << "<br><strong>Column Names:</strong></br><ul>";
                for (const std::string& result : sptr_database->results)
                {
                    beast::ostream(response_.body())
                        << "<li>" << result << "</li>\n";
                }
                beast::ostream(response_.body()) << "</ul>";
            }

            beast::ostream(response_.body())
                << "</body>\n"
                << "</html>\n";

            request_.target().clear();
        }
        else if (request_.target().find("/upload") == 0)
        {
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response /upload." << std::endl;
                #endif /* DEBUG_INFORMATION */
            
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<!DOCTYPE html>\n" // this is new remove
                << "<html>\n"
                << "<head>\n"
                << "<title> File Upload </title> \n"
                << "</head>\n"
                << "<body>\n"
                << "<h1> File upload </h1>\n"
                << "<p> <a href=\"/main\">Return to Main Page</a></p>\n"
                << "<form action=\"http://127.0.0.1:"
                << std::to_string(PORT)
                << "/upload\" method=\"post\" enctype=\"multipart/form-data\">\n"
                << "<input type=\"file\" name=\"upload-file\">\n"
                << "<input type=\"submit\" value=\"Upload\">\n"
                << "</form>\n"
                << "</body>\n"
                << "</html>\n";

            request_.target().clear();
        }
        else if (request_.target().find("/search") == 0)
        {
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response /search." << std::endl;
                #endif /* DEBUG_INFORMATION */

            response_.set(http::field::content_type, "text/html");

            if(!sptr_database->tables.empty())
            {
                beast::ostream(response_.body()) << generateDynamicResponse(sptr_database->results, IP_ADDRESS, PORT);
            }
            else
            {
                beast::ostream(response_.body()) << "Tables are empty. Nothing to search.\n";
                beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
            }

            request_.target().clear();
        }
        else if (request_.target().find("/showtable") == 0)
        {
            #ifdef __linux__
            std::string target_path = request_.target().substr(8).to_string();
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response /showtable" << std::endl;
                    std::cout << "target: " << request_.target().to_string()<< std::endl;
                    std::cout << "target_path: " << target_path << std::endl;
                #endif // DEBUG_INFORMATION
            #elif defined(_WIN32) || defined(_WIN64)
            std::string target_path = request_.target().substr(8);
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response /showtable" << std::endl;
                    std::cout << "target: " << request_.target()<< std::endl;
                    std::cout << "target_path: " << target_path << std::endl;
                #endif // DEBUG_INFORMATION
            #endif // __linux__ _WIN32 _WIN64

            if (!sptr_database->tables.empty())
            {
                size_t value_start = target_path.find("tableNumber=") + strlen("tableNumber=");
                size_t value_end = target_path.find("&", value_start);
                if (value_end == std::string::npos) {
                    // Если символ '&' не найден, значит, это последний параметр
                    value_end = target_path.length();
                }

                std::string table_number_value = target_path.substr(value_start, value_end - value_start);

                if(!table_number_value.empty())
                {
                    size_t table_id = std::stoi(table_number_value);

                    if (table_id <= sptr_database->countOfTables)
                    {
                        if (sptr_database->countOfTables == 1)
                        {
                            table_id = 0;
                            sptr_database->currentTableId = table_id;
                        }
                        if (table_id >= sptr_database->tables.size())
                        {
                            beast::ostream(response_.body()) << "Table with that big number was not found\n";
                            beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
                        }
                        else
                        {
                            beast::ostream(response_.body()) << generateDynamicResponse(sptr_database->tables[table_id], IP_ADDRESS, PORT);
                        }
                    }
                    else
                    {
                        sptr_database->currentTableId = 0;
                        // change to generateDynamicEmptyResponse(std::string & resp)
                        beast::ostream(response_.body()) << "Table with that number was not found.\n";
                        beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
                    }
                }
                else
                {
                        sptr_database->currentTableId = 0;
                        // change to generateDynamicEmptyResponse(std::string & resp)
                        beast::ostream(response_.body()) << "Table with that number was not found.\n";
                        beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
                }
            }
            else
            {
                response_.set(http::field::content_type, "text/html");
                // change to  generateDynamicEmptyTableResponse(std::string & resp
                beast::ostream(response_.body()) << "tables is empty. Cant show any data with /showtable. \n";
                beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
            }

            request_.target().clear();
        }
        else if (request_.target().find("/set") == 0)
        {
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response /set." << std::endl;
                #endif /* DEBUG_INFORMATION */
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
        }
        else if(request_.target().find("/favicon.ico") == 0)
        {
            // also nothing to do.
            #ifdef DEBUG_INFORMATION
                std::cout << "create_response /favicon.ico" << std::endl;
            #endif /* DEBUG_INFORMATION */
        }       
        else if(request_.target().find("/clean" == 0))
        {
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response /clean." << std::endl;
                #endif /* DEBUG_INFORMATION */
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << "<p>Data was cleared. Now you have 0 uploaded csv files.</p> <br>\n";
            beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
        }
        
        else
        {
                #ifdef DEBUG_INFORMATION
                    std::cout << "create_response empty." << std::endl;
                #endif /* DEBUG_INFORMATION */
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body()) << "Empty response. Please chose right option.\n";
            beast::ostream(response_.body()) << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
            request_.target().clear();
        }
    }
    // friend 
    friend void lines_eraser(const std::string& filename);
};

void http_server(tcp::acceptor& acceptor, tcp::socket& socket, std::shared_ptr<Database>& sptr_database)
{
    acceptor.async_accept(socket,
        [&](beast::error_code ec)
        {
            if (!ec)
                std::make_shared<http_connection>(std::move(socket), sptr_database)->start();
            http_server(acceptor, socket, sptr_database);
        });
}