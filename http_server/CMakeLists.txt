cmake_minimum_required(VERSION 3.20)

add_executable(HttpServerApp 
	main.cpp
	http_connection.h
	http_connection.cpp
	dataBaseSearch.h
	dataBaseSearch.cpp)

target_compile_features(HttpServerApp PRIVATE cxx_std_17) 

target_link_libraries(HttpServerApp pqxx)  

target_include_directories(HttpServerApp PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(HttpServerApp ${Boost_LIBRARIES})

target_link_libraries(HttpServerApp OpenSSL::SSL)

target_include_directories(HttpServerApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(HttpServerApp ParserLib)

set(INI_SOURCE ../config/settings.ini)
set(INI_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config)
file(COPY ${INI_SOURCE} DESTINATION ${INI_DESTINATION})
