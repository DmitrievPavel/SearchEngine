#pragma once 

#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <string>

class INI_Parser
{
private:
	std::unordered_map<std::string, std::unordered_map<std::string, std::string>> data;
	std::string get_values_name(std::string& section_name);
public:
	INI_Parser();
	~INI_Parser() = default;
	void parse(const std::string& filename);
	template<class T>
	T get_value(const std::string& key)
	{
		size_t dot = key.find_first_of('.');
		std::string section_name = key.substr(0, dot);
		std::string value_name = key.substr(dot + 1, key.length() - 1);

		if (data.count(section_name) > 0 && data.at(section_name).count(value_name) > 0)
		{
			std::string value = data[section_name][value_name];

			if constexpr (std::is_same_v<T, int>)
			{
				return std::stoi(value);
			}
			else if constexpr (std::is_same_v<T, double>)
			{
				return std::stod(value);
			}
			else if constexpr (std::is_same_v<T, std::string>)
			{
				return value;
			}
			else if constexpr (std::is_same_v<T, bool>)
			{
				return (value != "0");
			}
			else
			{
				throw std::runtime_error("Parser:error type");
			}
		}
		else
		{
			throw std::runtime_error("Parser:value not found for this section\nValues for this section:\n" + get_values_name(section_name));
		}
	}
};