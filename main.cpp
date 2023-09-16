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

#include "database.h"
#include "http_connection.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

const std::string port_const = "7777";
const std::string ip_address_const = "127.0.0.1";

int main(int argc, char* argv[])
{
    Database database_;
    std::shared_ptr<Database> sptr_database_ = std::make_shared<Database>(database_);
    
    if (argc != 3)
    {
        argc = 3;
        argv[0] = (char*)"receiver";
        argv[1] = (char*)ip_address_const.c_str();
        argv[2] = (char*)port_const.c_str();
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

        auto const address = net::ip::make_address(ip_address_const); // auto const address = net::ip::make_address(argv[1]);
        unsigned short port = static_cast<unsigned short>(std::atoi(port_const.c_str())); //unsigned short port = static_cast<unsigned short>(std::atoi(argv[2]));

        net::io_context ioc{ 1 };

        tcp::acceptor acceptor{ ioc, {address, port} };
        tcp::socket socket{ ioc };
        http_server(acceptor, socket, sptr_database_, ip_address_const ,port_const);

        ioc.run();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}




