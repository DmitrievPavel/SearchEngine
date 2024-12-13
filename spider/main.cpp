#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <map>

#include "http_utils.h"
#include <functional>
#include "parser.h"
#include "dataBase.h"


std::mutex mtx;
std::condition_variable cv;
std::queue<std::function<void()>> tasks;
bool exitThreadPool = false;


void threadPoolWorker() {
	std::unique_lock<std::mutex> lock(mtx);
	while (!exitThreadPool || !tasks.empty()) {
		if (tasks.empty()) {
			cv.wait(lock);
		}
		else {
			auto task = tasks.front();
			tasks.pop();
			lock.unlock();
			task();
			lock.lock();
		}
	}
}
void parseLink(const Link& link, int depth, DataBase& db)
{
	try {

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		std::string html = getHtmlContent(link);

		if (html.size() == 0)
		{
			std::cout << "Failed to get HTML Content" << std::endl;
			return;
		}

		std::vector<Link> links = getLinks(html, link);
		std::map<std::string, int> words = getWordsMap(html);

		std::string url = link.hostName + link.query;

		if (db.addLink(url))
		{
			for (const auto& word : words)
			{
				db.addWordCount(url, word.first, word.second);
			}
		}

		if (depth > 0)
		{
			std::lock_guard<std::mutex> lock(mtx);

			for (auto& subLink : links)
			{
				tasks.push([subLink, depth, &db]() { parseLink(subLink, depth - 1, db); });
			}
			cv.notify_one();
		}
	}
	catch (const pqxx::sql_error& e)
	{
		std::cout << e.what();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

}



int main()
{
	try {

		INI_Parser parser;
		parser.parse("settings.ini");

		std::string conString = 
			"host=" + parser.get_value<std::string>("DataBase.host") +
			" port=" + parser.get_value<std::string>("DataBase.port") +
			" dbname=" + parser.get_value<std::string>("DataBase.dbname") +
			" user=" + parser.get_value<std::string>("DataBase.user") +
			" password=" + parser.get_value<std::string>("DataBase.password");
		
		DataBase db(conString);
		db.connect();
		db.createTables();
		int depth = parser.get_value<int>("Spider.depth");
		std::string startLink = parser.get_value<std::string>("Spider.startlink");
		std::string host = startLink.substr(0, startLink.find("/"));
		std::string query = startLink.substr(startLink.find("/"), startLink.size());




		int numThreads = std::thread::hardware_concurrency();
		std::vector<std::thread> threadPool;

		for (int i = 0; i < numThreads; ++i) {
			threadPool.emplace_back(threadPoolWorker);
		}

		Link link{ ProtocolType::HTTPS, host, query };


		{
			std::lock_guard<std::mutex> lock(mtx);
			tasks.push([link, depth, &db]() { parseLink(link, depth, db); });
			cv.notify_one();
		}


		std::this_thread::sleep_for(std::chrono::seconds(2));


		{
			std::lock_guard<std::mutex> lock(mtx);
			exitThreadPool = true;
			cv.notify_all();
		}

		for (auto& t : threadPool) {
			t.join();
		}

	}
	catch (const pqxx::sql_error& e)
	{
		std::cout << e.what();
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}

	catch (...)
	{
		std::cout << "Unknown error!";
	}
	return 0;
}
