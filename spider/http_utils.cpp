#include "http_utils.h"

#include <regex>
#include <iostream>

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <openssl/ssl.h>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/locale.hpp>
#include <map>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
namespace ip = boost::asio::ip;
namespace ssl = boost::asio::ssl;

using tcp = boost::asio::ip::tcp;

bool isText(const boost::beast::multi_buffer::const_buffers_type& b)
{
	for (auto itr = b.begin(); itr != b.end(); itr++)
	{
		for (int i = 0; i < (*itr).size(); i++)
		{
			if (*((const char*)(*itr).data() + i) == 0)
			{
				return false;
			}
		}
	}

	return true;
}

std::string getHtmlContent(const Link& link)
{

	std::string result;

	try
	{
		std::string host = link.hostName;
		std::string query = link.query;

		net::io_context ioc;

		if (link.protocol == ProtocolType::HTTPS)
		{

			ssl::context ctx(ssl::context::tlsv13_client);
			ctx.set_default_verify_paths();

			beast::ssl_stream<beast::tcp_stream> stream(ioc, ctx);
			stream.set_verify_mode(ssl::verify_none);

			stream.set_verify_callback([](bool preverified, ssl::verify_context& ctx) {
				return true; // Accept any certificate
				});


			if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str())) {
				beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
				throw beast::system_error{ec};
			}

			ip::tcp::resolver resolver(ioc);
			get_lowest_layer(stream).connect(resolver.resolve({ host, "https" }));
			get_lowest_layer(stream).expires_after(std::chrono::seconds(30));


			http::request<http::empty_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

			stream.handshake(ssl::stream_base::client);
			http::write(stream, req);

			beast::flat_buffer buffer;
			http::response<http::dynamic_body> res;
			http::read(stream, buffer, res);

			if (isText(res.body().data()))
			{
				result = buffers_to_string(res.body().data());
			}
			else
			{
				std::cout << "This is not a text link, bailing out..." << std::endl;
			}

			beast::error_code ec;
			stream.shutdown(ec);
			if (ec == net::error::eof) {
				ec = {};
			}

			if (ec) {
				throw beast::system_error{ec};
			}
		}
		else
		{
			tcp::resolver resolver(ioc);
			beast::tcp_stream stream(ioc);

			auto const results = resolver.resolve(host, "http");

			stream.connect(results);

			http::request<http::string_body> req{http::verb::get, query, 11};
			req.set(http::field::host, host);
			req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);


			http::write(stream, req);

			beast::flat_buffer buffer;

			http::response<http::dynamic_body> res;


			http::read(stream, buffer, res);

			if (isText(res.body().data()))
			{
				result = buffers_to_string(res.body().data());
			}
			else
			{
				std::cout << "This is not a text link, bailing out..." << std::endl;
			}

			beast::error_code ec;
			stream.socket().shutdown(tcp::socket::shutdown_both, ec);

			if (ec && ec != beast::errc::not_connected)
				throw beast::system_error{ec};

		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
	}

	return result;
}

std::map<std::string, int> getWordsMap(const std::string& html)
{

	std::string text = html;
	std::regex tagPattern("<[^>]*>");
	text = std::regex_replace(text, tagPattern, " ");

	std::replace_if(text.begin(), text.end(), [](char c) {
		return !std::isalnum(static_cast<unsigned char>(c));
		}, ' ');

	std::transform(text.begin(), text.end(), text.begin(),
		[](unsigned char c) { return std::tolower(c); });

	std::map<std::string, int> frequency;
	std::vector<std::string> words;
	boost::algorithm::split(words, text, boost::algorithm::is_space());

	for (auto& word : words) {

		if (word.length() >= 3 && word.length() <= 32) {
			frequency[word]++;
		}
	}
	return frequency;
}

std::vector<Link> getLinks(const std::string& html, const Link& HomeLink)
{
	std::vector<Link> links;
	const std::string prefix_http = "http://";
	const std::string prefix_https = "https://";

	boost::regex hrefRegex(R"(href="((?:https?://|//|/))([^"]+))");
	boost::sregex_iterator iter(html.begin(), html.end(), hrefRegex);
	boost::sregex_iterator end;

	boost::regex linkRegex("([^/]+)(/.*)");
	boost::smatch match;

	while (iter != end) {
		std::string pref = (*iter)[1];
		std::string host = (*iter)[2];
		std::string link = (*iter)[2];
		std::string path = "/";
		if (link.find("File") != std::string::npos)
		{
			iter++;
			continue;
		}
		Link NewLink;

		if (pref == "/")
		{
			NewLink = { HomeLink.protocol, HomeLink.hostName, "/" + link };
		}
		else 
		{
			if (boost::regex_match(link, match, linkRegex))
			{
				if (match.size() == 3) {
					host = match[1];
					path = match[2];
				}
			}
			if (pref == "//" || pref == prefix_https) 
			{
				NewLink = { ProtocolType::HTTPS, host, path };
			}
			else if (pref == prefix_http) 
			{
				NewLink = { ProtocolType::HTTP, host, path };
			}
		}
		iter++;

		if (links.size() > 30)
		{
			iter = end;
			continue;
		}

		auto it = std::find_if(links.begin(), links.end(), [&](const Link& el) 
			{
			return el == NewLink;
			});
		if (it == links.end()) 
		{
			links.push_back(NewLink);
		}
	}
	return links;

}



