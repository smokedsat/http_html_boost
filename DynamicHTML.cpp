#include "DynamicHTML.h"

// HTML with vector<std::string> result
std::string 
    generateDynamicResponse(const std::vector<std::string>& data, const std::string & ip_address, const std::string & port)
{
    std::stringstream html;
    html << "<!DOCTYPE html>\n";
    html << "<html>\n";
    html << "<head>\n";
    html << "    <title>RESULTS</title>\n";
    html << "</head>\n";
    html << "<body>\n";
    html << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
    html << "<form action=\"http://";
    html << ip_address;
    html << ":";
    html << port;
    html << "\">";
    if (data.empty())
    {
        html << "<h1>Nothing to show</h1>\n";
    }
    else
    {
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
    generateDynamicResponse(const Table& table, const std::string& ip_address, const std::string& port)
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
            for (auto i = 0; i < rows.size() - 1; i++) {
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
        html << "<p><strong> <a href=\"/main\">Return to Main Page</a></strong></p>\n";
        html << "<form action=\"http://";
        html << ip_address;
        html << ":";
        html << port;
        html << "\">";
        html << "</body>\n";
        html << "</html>\n";

        return html.str();
    }
    else
    {
        return "";
    }
}
