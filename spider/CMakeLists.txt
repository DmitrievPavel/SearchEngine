cmake_minimum_required(VERSION 3.20)

add_executable(SpiderApp
	main.cpp
	http_utils.h
	http_utils.cpp
	link.h
	 dataBase.h
	 dataBase.cpp)

target_compile_features(SpiderApp PRIVATE cxx_std_17) 

target_link_libraries(SpiderApp pqxx)

target_link_libraries(SpiderApp OpenSSL::SSL)

target_include_directories(SpiderApp PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(SpiderApp ${Boost_LIBRARIES})

target_include_directories(SpiderApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SpiderApp ParserLib)

set(INI_SOURCE ../config/settings.ini)
set(INI_DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/config)
file(COPY ${INI_SOURCE} DESTINATION ${INI_DESTINATION})