#pragma once
#include <pqxx/pqxx>
#include <iostream>

using UrlCount = std::pair<std::string, int>;

class DataBaseSearch {
private:
	pqxx::connection* db = nullptr;
public:
	DataBaseSearch();
	DataBaseSearch(const DataBaseSearch& other) = delete;
	DataBaseSearch& operator=(const DataBaseSearch& other) = delete;
	DataBaseSearch(DataBaseSearch&& other) noexcept;
	DataBaseSearch& operator=(DataBaseSearch&& other) noexcept;
	~DataBaseSearch();

	void connect(const std::string& connectionStr);
	void completeRequest(std::string& req, std::vector<std::string>& searchResult);
};