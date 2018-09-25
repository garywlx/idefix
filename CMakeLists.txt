# Require cmake version
cmake_minimum_required(VERSION 3.7)

# Change this to your project name
project(idefix LANGUAGES CXX)

# set version MAJOR.MINOR.PATCH
set(PROJECT_VERSION 0.2.0)

# set compiler
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# set directories of custom includes
set(inc_QUICKFIX "/usr/local/include/quickfix")
set(inc_PROJECT_DIR "include")
set(inc_SPDLOG "include/spdlog")

# include directories
include_directories(src ${inc_QUICKFIX} ${inc_PROJECT_DIR})

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

add_compile_options("-Wno-c++98-compat")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${inc_PROJECT_DIR} -I${inc_QUICKFIX} -fexceptions")

message(STATUS "Build Type is: ${CMAKE_BUILD_TYPE}")

set(SRC src/main.cpp 
	src/RiskManagement.h 
	src/MathHelper.h 
	src/Pairs.h 
	src/MarketDetail.h 
	src/FXCMFields.h 
	src/FIXFactory.h 
	src/times.h 
	src/StringHelper.h 
	src/MarketOrder.h 
	src/MarketSnapshot.h 
	src/Market.h 
	src/Operators.h 
	src/Exceptions.h 
	src/SignalType.h 
	src/MarketSide.h
	src/Account.h 
	src/Account.cpp 
	src/FIXManager.h 
	src/FIXManager.cpp 
	src/RequestId.h 
	src/RequestId.cpp 
	src/RenkoChart.h 
	src/RenkoChart.cpp 
	src/AwesomeStrategy.h 
	src/AwesomeStrategy.cpp
	src/CSVHandler.h 
	src/CSVHandler.cpp
	src/Bar.h
	src/RenkoBrick.h 
	src/AbstractMovingAverage.h
	src/SimpleMovingAverage.h 
	src/SimpleMovingAverage.cpp 
)

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# add version definition 
target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_PROJECT_VERSION="${PROJECT_VERSION}")

# set build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_RELEASE_LOG=1)
else()
	set(CMAKE_BUILD_TYPE Debug)
endif()

# linker
if(APPLE)
	set(LINK_LIBS /usr/local/lib/libquickfix.dylib)
elseif(NOT APPLE)
	find_package(Threads REQUIRED)
	set(LINK_LIBS /usr/local/lib/libquickfix.so ${CMAKE_THREAD_LIBS_INIT})
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(STATUS "POST_BUILD copy ${PROJECT_NAME} binary to ${CMAKE_CURRENT_SOURCE_DIR}/build/")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# copy fxcm cfg file to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/fxcm.cfg ${CMAKE_CURRENT_SOURCE_DIR}/build/)
	message(STATUS "POST_BUILD copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/fxcm.cfg to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
	# copy fixfxcm10.xml to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/FIXFXCM10.xml ${CMAKE_CURRENT_SOURCE_DIR}/build/)
	message(STATUS "POST_BUILD copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/FIXFXCM10.xml to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
	# create folder build/public_html
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html)
	message(STATUS "POST_BUILD make directory ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html/")
	# copy chart.html to parent directory build/public_html
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/html/chart.html ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html)
	message(STATUS "POST_BUILD copy ${CMAKE_CURRENT_SOURCE_DIR}/src/html/chart.html to ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html")
endif()

# install idefix in /usr/local/bin
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
