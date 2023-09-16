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

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;


void 
    http_server(tcp::acceptor& acceptor, tcp::socket& socket, std::shared_ptr<Database>& sptr_database, const std::string & ip_address ,const std::string & port);

class 
    http_connection : public std::enable_shared_from_this<http_connection>
{
public:
    http_connection(tcp::socket socket, std::shared_ptr<Database>& sptr_database, const std::string & port, const std::string & ip_address);

    void start();

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{ 8192 };
    http::request<http::dynamic_body> request_;
    http::response<http::dynamic_body> response_;
    net::steady_timer deadline_{ socket_.get_executor(), std::chrono::seconds(60) };

    std::shared_ptr<Database> sptr_database;

    const std::string PORT;
    const std::string IP_ADDRESS;
private:
    void read_request();
    void process_request();

    void check_deadline();

    void create_response();
    void write_response();

    // friend 
    friend void lines_eraser(const std::string& filename);
};




