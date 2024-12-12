cmake_minimum_required(VERSION 3.20)

project(CppDiplomProject)


# TODO: Define by your own needs:
set(OPENSSL_ROOT_DIR "D:/OpenSSL-Win64")
set (CMAKE_CXX_STANDARD 20) 
set (Boost_NO_SYSTEM_PATHS ON)
set (Boost_USE_MULTITHREADED ON)
set (Boost_USE_STATIC_LIBS ON)
set (Boost_USE_STATIC_RUNTIME OFF)
set (BOOST_ALL_DYN_LINK OFF)

set(BOOST_ROOT "D:/boost_1_86_0")

set(libpqxx_DIR "D:/libpqxx-7.9.2")


# TODO: If you are running on Windows below 10, change this:

add_compile_definitions(_WIN32_WINNT=0x0A01)



# Boost:
find_package(Boost 1.80.0 REQUIRED COMPONENTS system thread locale)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find Boost")
endif()


# PQXX
set(PostgreSQL_ADDITIONAL_VERSIONS "15")

add_subdirectory(${libpqxx_DIR} libpqxx-build) 


# OpenSSL
find_package(OpenSSL REQUIRED)



add_subdirectory(spider)

add_subdirectory(http_server)

add_subdirectory(parser)
