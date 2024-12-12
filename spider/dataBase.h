#pragma once
#include <pqxx/pqxx>
#include <iostream>
#include <mutex>

class DataBase {
private:
	pqxx::connection* db = nullptr;
	std::string connectionStr;
	std::mutex mtx;
public:
	DataBase(const std::string& conStr) : connectionStr(conStr) {};
	DataBase() = delete;
	DataBase(const DataBase& other) = delete;
	DataBase& operator=(const DataBase& other) = delete;
	DataBase(DataBase&& other) noexcept;
	DataBase& operator=(DataBase&& other) noexcept;
	~DataBase();

	void connect();
	void createTables();
	bool addLink(const std::string& url);
	void addWordCount(const std::string& url, const std::string& word, const int& count);
};