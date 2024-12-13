#include "dataBaseSearch.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string_regex.hpp>
#include <map>

std::vector<std::string> prepareReq(std::string& req)
{
	boost::algorithm::to_lower(req, std::locale());
	boost::algorithm::replace_all_regex(req, boost::regex("[[:punct:]]"), std::string{ " " });
	boost::trim_if(req, boost::is_punct(std::locale()));
	boost::algorithm::replace_all_regex(req, boost::regex("\\ {2,}"), std::string{ " " });
	std::vector<std::string> words;
	boost::algorithm::split(words, req, boost::algorithm::is_space());
	return words;
}

DataBaseSearch::DataBaseSearch()
{
}

DataBaseSearch::DataBaseSearch(DataBaseSearch&& other) noexcept
{
	db = other.db;
	other.db = nullptr;
}

DataBaseSearch& DataBaseSearch::operator=(DataBaseSearch&& other) noexcept
{
	db = other.db;
	other.db = nullptr;
	return *this;
}

DataBaseSearch::~DataBaseSearch()
{
	delete db;
}

void DataBaseSearch::connect(const std::string& connectionStr)
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

void DataBaseSearch::completeRequest(std::string& req, std::vector<std::string>& searchResult)
{
	try
	{
		searchResult.clear();
		pqxx::work tr{ *db };
		std::vector<std::string> reqWords = prepareReq(req);
		std::map<std::string, std::map<std::string, int>> urls;
		int countWords = reqWords.size();
		for (const auto& word : reqWords)
		{
			pqxx::result table = tr.exec("select url, w.quantity from documents d join words w on w.url_id = d.id where word = '" + tr.esc(word) + "'");
			for (const auto& row : table)
			{
				urls[row["url"].as<std::string>()][word] = row["quantity"].as<int>();
			}
		}

		std::vector<UrlCount> result;
		for (const auto& url : urls)
		{
			int count = 0;
			if (url.second.size() == countWords)
			{
				for (const auto& w : url.second)
				{
					count += w.second;
				}
				result.push_back(std::make_pair(url.first, count));
			}
		}
		std::sort(result.begin(), result.end(), [](const UrlCount& a, const UrlCount& b) {return a.second > b.second; });
		for (const auto& url : result)
		{
			searchResult.push_back(url.first);
		}
	}
	catch (const std::exception& ex)
	{
		std::cout << ex.what();
	}
}
