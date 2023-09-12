#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <filesystem>

#include "csv_reader.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>

const std::string file = "clear_uploaded_file.csv";

void erase_4_1_lines(const std::string& filename);

namespace my_program_state
{
    std::size_t
        request_count()
    {
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t
        now()
    {
        return std::time(0);
    }
}

class http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    

    http_connection(tcp::socket socket)
        : socket_(std::move(socket))
    {
    }

    // Initiate the asynchronous operations associated with the connection.
    void
        start()
    {
        read_request();
        check_deadline();
    }

private:
    // The socket for the currently connected client.
    tcp::socket socket_;

    // The buffer for performing reads.
    beast::flat_buffer buffer_{ 8192 };

    // The request message.
    http::request<http::dynamic_body> request_;

    // The response message.
    http::response<http::dynamic_body> response_;

    // The timer for putting a deadline on connection processing.
    net::steady_timer deadline_{
        socket_.get_executor(), std::chrono::seconds(60) };

    // Temporary table from csv
    Table table;

    // Asynchronously receive a complete request message.
    void
        read_request()
    {
        auto self = shared_from_this();

        http::async_read(
            socket_,
            buffer_,
            request_,
            [self](beast::error_code ec,
                std::size_t bytes_transferred)
            {
                boost::ignore_unused(bytes_transferred);
                if (!ec)
                    self->process_request();
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
            if (request_.target() == "/main")
            {
                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");
                response_.prepare_payload();
                create_response();
            }
            else if (request_.target() == "/readcsv")
            {
                table.readCSV(file);

                response_.result(http::status::ok);
                response_.set(http::field::server, "Get request");

                beast::ostream(response_.body()) << generateDynamicResponse(table);
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
            if (request_.body().size() > 0)
            {
                std::string file_data = beast::buffers_to_string(request_.body().data());

                const std::string filename = "uploaded_file.csv";

                std::ofstream outfile(filename);
                outfile << file_data;
                outfile.close();

                erase_4_1_lines(filename);
                

                response_.result(http::status::ok);
                response_.set(http::field::server, "Post request");
                beast::ostream(response_.body()) << "File was successfully uploaded.\n";
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
        if (request_.target() == "/count")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been "
                << my_program_state::request_count()
                << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (request_.target() == "/readcsv")
        {
            std::string filename = "clear_uploaded_file.csv";
            table.readCSV(filename);
            response_.set(http::field::content_type, "text/html");
            //beast::ostream(response_.body()) << generateDynamicResponse(table);
        }
        else if (request_.target() == "/time")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is "
                << my_program_state::now()
                << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else if (request_.target() == "/upload")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head>\n"
                <<  "<title> File Upload </title> \n"
                <<  "</head>\n"
                <<  "<body>\n"
                <<      "<h1> File upload </h1>\n"
                <<      "<p> <a href=\"/main\">Return to Main Page</a></p>\n"
                <<      "<form action=\"http://127.0.0.1:7777/upload\" method=\"post\" enctype=\"multipart/form-data\">\n"
                <<          "<input type=\"file\" name=\"upload-file\">\n"
                <<          "<input type=\"submit\" value=\"Upload\">\n"
                <<      "</form>\n"
                <<  "</body>\n"
                << "</html>\n";

            std::string filename = "clear_uploaded_file.csv";
            table.readCSV(filename);
        }
        else if (request_.target() == "/main")
        {
            response_.set(http::field::content_type, "text/html");
            beast::ostream(response_.body())
                << "<html>\n"
                << "<head><title>Main Page</title></head>\n"
                << "<body>\n"
                << "<h1>Main Page</h1>\n"
                << "<p>Welcome to the main page. Choose an action:</p>\n"
                << "<ul>\n"
                << "<li><a href=\"/count\">Request count</a></li>\n"
                << "<li><a href=\"/time\">Current time</a></li>\n"
                << "<li><a href=\"/upload\">Upload a file</a></li>\n"
                << "<li><a href=\"/readcsv\">Read csv</a></li>\n"
                << "</ul>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else
        {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "File not found\r\n";
        }
    }

    // Asynchronously transmit the response message.
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


    // Check whether we have spent enough time on this connection.
    void
        check_deadline()
    {
        auto self = shared_from_this();

        deadline_.async_wait(
            [self](beast::error_code ec)
            {
                if (!ec)
                {
                    // Close socket to cancel any outstanding operation.
                    self->socket_.close(ec);
                }
            });
    }

    std::string 
        generateDynamicResponse(const Table& table)
    {
        if (!this->table.table.empty())
        {
            std::stringstream html;
            html << "<!DOCTYPE html>\n";
            html << "<html>\n";
            html << "<head>\n";
            html << "    <title>Ðåçóëüòàòû èç òàáëèöû</title>\n";
            html << "</head>\n";
            html << "<body>\n";
            html << "    <h1>Äàííûå èç òàáëèöû</h1>\n";
            html << "    <table>\n";
            html << "        <tr>\n";

            for (const std::string& column_type : table.column_types) {
                html << "            <th>" << column_type << "</th>\n";
            }

            for (const std::string& column_name : table.column_names) {
                html << "            <th>" << column_name << "</th>\n";
            }

            html << "        </tr>\n";

            for (const auto& rows : table.table) {
                html << "        <tr>\n";
                for (auto i = 0; i < rows.size(); i++) {
                    if (!rows[i].empty()) { html << "            <td>" << rows[i] << "</td>\n"; }
                    else {html << "            <td></td>\n"; }
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
};

// "Loop" forever accepting new connections.
void
http_server(tcp::acceptor& acceptor, tcp::socket& socket)
{
    acceptor.async_accept(socket,
        [&](beast::error_code ec)
        {
            if (!ec)
                std::make_shared<http_connection>(std::move(socket))->start();
            http_server(acceptor, socket);
        });
}



void erase_4_1_lines(const std::string& filename)
{
    std::ifstream inputFile(filename);
    std::ofstream outputFile("clear_" + filename);

    if (!inputFile.is_open() || !outputFile.is_open()) {
        std::cerr << "Failed to open files." << std::endl;
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

    for(auto i =0 ; i <=3 ; i++)
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


int
main(int argc, char* argv[])
{
    if (argc != 3)
    {
        argc = 3;
        argv[0] = (char*)"receiver";
        argv[1] = (char*)"127.0.0.1";
        argv[2] = (char*)"7777";
    }
    try
    {
        // Check command line arguments.
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
        http_server(acceptor, socket);

        ioc.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
// ______________________________________________________________________________

