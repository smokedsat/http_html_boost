#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <iostream>
#include <fstream>

void use()
{
    boost::property_tree::ptree pt;

    // Запись данных в JSON
    pt.put("name", "John");
    pt.put("age", 30);
    pt.put("city", "New York");

    // Запись JSON в строку
    std::ostringstream json_data;
    boost::property_tree::write_json(json_data,pt);

    std::cout << json_data.str() << std::endl;

}