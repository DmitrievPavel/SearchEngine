#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include "http_connection.h"
#include <Windows.h>
#include "parser.h"


void httpServer(tcp::acceptor& acceptor, tcp::socket& socket,const std::string& conStr )
{
	acceptor.async_accept(socket,
		[&](beast::error_code ec)
		{
			if (!ec)
				std::make_shared<HttpConnection>(std::move(socket))->start(conStr);
			httpServer(acceptor, socket,conStr);
		});
}


int main(int argc, char* argv[])
{
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);

	try
	{
		INI_Parser parser;
		parser.parse("settings.ini");

		std::string conString =
			"host=" + parser.get_value<std::string>("DataBase.host") +
			" port=" + parser.get_value<std::string>("DataBase.port") +
			" dbname=" + parser.get_value<std::string>("DataBase.dbname") +
			" user=" + parser.get_value<std::string>("DataBase.user") +
			" password=" + parser.get_value<std::string>("DataBase.password");

		auto const address = net::ip::make_address("0.0.0.0");
		unsigned short port = parser.get_value<int>("Server.port");

		net::io_context ioc{1};

		tcp::acceptor acceptor{ioc, { address, port }};
		tcp::socket socket{ioc};
		httpServer(acceptor, socket, conString);

		std::cout << "Open browser and connect to http://localhost:" << port << " to see the web server operating" << std::endl;

		ioc.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
}