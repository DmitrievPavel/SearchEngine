#include "dataBase.h"

DataBase::DataBase(DataBase&& other) noexcept
{
	connectionStr = other.connectionStr;
	db = other.db;
	other.db = nullptr;
}

DataBase& DataBase::operator=(DataBase&& other) noexcept
{
	connectionStr = other.connectionStr;
	db = other.db;
	other.db = nullptr;
	return *this;
}

DataBase::~DataBase()
{
	delete db;
}

void DataBase::connect()
{
	try
	{
		db = new pqxx::connection(connectionStr);
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what();
	}
}

void DataBase::createTables()
{

	pqxx::work tr{ *db };

	tr.exec(
		"CREATE table IF NOT EXISTS documents ( "
		"id serial primary KEY, "
		"url text NOT NULL );");

	tr.exec(
		"CREATE table IF NOT EXISTS words ( "
		"id serial primary KEY, "
		"word varchar(32) NOT NULL, "
		"quantity integer NOT NULL,"
		"url_id integer references documents(id));");

	tr.commit();

}


bool DataBase::addLink(const std::string& url)
{
	mtx.lock();
	pqxx::work tr{ *db };
	auto queryRes = tr.exec("SELECT url FROM documents WHERE url ='" + tr.esc(url) + "'");
	if (queryRes.empty())
	{
		tr.exec("INSERT INTO documents (url) VALUES ('" + tr.esc(url) + "')");
		tr.commit();
		std::cout << url << " parsed" << std::endl;
		mtx.unlock();
		return true;
	}
	tr.commit();
	mtx.unlock();
	return false;
}

void DataBase::addWordCount(const std::string& url, const std::string& word, const int& count)
{
	mtx.lock();
	pqxx::work tr{ *db };

	int urlId = tr.query_value<int>("SELECT id FROM documents WHERE url ='" + tr.esc(url) + "'");
	tr.exec("INSERT INTO words(word,quantity,url_id) VALUES ('" + tr.esc(word) + "','" + tr.esc(std::to_string(count)) + "','" + tr.esc(std::to_string(urlId)) + "')");
	tr.commit();
	mtx.unlock();
}
