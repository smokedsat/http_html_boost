#include <boost/log/trivial.hpp>
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/console.hpp>

namespace logging = boost::log;

void init_logger() {
    logging::register_simple_formatter_factory<logging::trivial::severity_level, char>("Severity");
    logging::add_console_log(std::cout,
        logging::keywords::format = "[%TimeStamp%] [%Severity%] %Message%"
    );
    logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
}

int main() {
    init_logger();

    BOOST_LOG_TRIVIAL(trace) << "This is a trace message";
    BOOST_LOG_TRIVIAL(debug) << "This is a debug message";
    BOOST_LOG_TRIVIAL(info) << "This is an info message";
    BOOST_LOG_TRIVIAL(warning) << "This is a warning message";
    BOOST_LOG_TRIVIAL(error) << "This is an error message";
    BOOST_LOG_TRIVIAL(fatal) << "This is a fatal message";

    return 0;
}