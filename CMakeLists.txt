cmake_minimum_required(VERSION 3.10.0)

# Change this to your project name
project(idefix LANGUAGES CXX)

# set build type
if(NOT CMAKE_BUILD_TYPE) 
	set(CMAKE_BUILD_TYPE Debug)
endif()

# add include directory with 3rd party libraries
include_directories(${CMAKE_HOME_DIRECTORY}/include)

# try to find quickfix in /usr/local/include
find_path(QUICKFIX_INCLUDE_PATH quickfix/FixFields.h)
find_library(QUICKFIX_LIBRARY_PATH quickfix)
if(QUICKFIX_INCLUDE_PATH AND QUICKFIX_LIBRARY_PATH)
	message(STATUS "Found quickfix in ${QUICKFIX_INCLUDE_PATH}")
	include_directories(${QUICKFIX_INCLUDE_PATH})
else()
	message(FATAL_ERROR "quickfix not found.")
endif()

# try to find spdlog in /usr/local/include
find_path(SPDLOG_INCLUDE_PATH spdlog/spdlog.h)
if(SPDLOG_INCLUDE_PATH)
	message(STATUS "Found spdlog in ${SPDLOG_INCLUDE_PATH}")
	include_directories(${SPDLOG_INCLUDE_PATH})
else()
	message(FATAL_ERROR "spdlog not found.")
endif()

# try to find boost
find_package(Boost REQUIRED COMPONENTS system thread program_options date_time filesystem iostreams coroutine context regex)
include_directories({Boost_INCLUDE_DIRS})

# set c++ standard
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

# set CXX flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-c++98-compat -fexceptions")

# add build/adapter directory
set(ADAPTER_BUILD_DIRECTORY "${CMAKE_HOME_DIRECTORY}/build/adapter")
file(MAKE_DIRECTORY ${ADAPTER_BUILD_DIRECTORY})

# add source directory
add_subdirectory(src)
