#include "parser.h"

void trim(std::string& str)
{
    size_t start = str.find_first_not_of(" \t\r\n");
    size_t end = str.find_last_not_of(" \t\r\n");

    if (start != std::string::npos && end != std::string::npos)
    {
        str = str.substr(start, end - start + 1);
    }

    else
    {
        str.clear();
    }
}

void trim_comment_to_value(std::string& value)
{
    size_t pos = value.find(';');
    if (pos != std::string::npos)
    {
        value = value.substr(0, pos);
    }
}

std::string INI_Parser::get_values_name(std::string& section_name)
{
    std::string name_list;
    for (auto& elem : data[section_name])
    {
        name_list += elem.first + '\n';
    }
    return name_list;
}

INI_Parser::INI_Parser()
{

}

void INI_Parser::parse(const std::string& file_name)
{
    std::ifstream file(file_name);

    if (!file.is_open())
    {
        throw std::runtime_error("file is not opened");
    }

    std::string line;
    std::string section_name;

    while (std::getline(file, line))
    {
        trim(line);

        if (line.empty() || line[0] == ';')
        {
            continue;
        }

        if (line[0] == '[' && line[line.length() - 1] == ']')
        {
            section_name = line.substr(1, line.length() - 2);
            trim(section_name);
            continue;
        }

        std::string value_name;
        std::string value;
        std::istringstream ss(line);

        if (std::getline(ss, value_name, '=') && std::getline(ss, value))
        {
            trim(value_name);
            trim(value);
            trim_comment_to_value(value);
            data[section_name][value_name] = value;
        }

    }
    file.close();
}
