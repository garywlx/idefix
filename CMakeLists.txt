# Require cmake version
cmake_minimum_required(VERSION 3.7)

# Change this to your project name
project(idefix LANGUAGES CXX)

# set version MAJOR.MINOR.PATCH
set(PROJECT_VERSION 0.2.8)

# set compiler
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug)
endif()

# find boost packages
find_package(Boost REQUIRED COMPONENTS system thread program_options date_time filesystem iostreams coroutine context regex) 
include_directories({Boost_INCLUDE_DIRS})

set(inc_PROJECT_DIR "include")
#set(inc_SPDLOG "include/spdlog")

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
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${inc_PROJECT_DIR} -I${inc_QUICKFIX} -fexceptions -lcrypto")

# set build type
if(CMAKE_BUILD_TYPE STREQUAL "Release")
	target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_RELEASE_LOG=1)
endif()

message(STATUS "Build Type is: ${CMAKE_BUILD_TYPE}")

# find all cpp files 
file(GLOB_RECURSE SRC src/*.cpp)

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# add version definition 
target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_PROJECT_VERSION="${PROJECT_VERSION}")
# uncomment if you want to show debug output on console()
target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_SHOW_DEBUG_OUTPUT=1)

# linker
if(APPLE)
	set(LINK_LIBS /usr/local/lib/libquickfix.dylib)
elseif(NOT APPLE)
	find_package(Threads REQUIRED)
	set(LINK_LIBS /usr/local/lib/libquickfix.so ${CMAKE_THREAD_LIBS_INIT})
endif()
# link
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS} ${Boost_LIBRARIES})

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(STATUS "POST_BUILD copy ${PROJECT_NAME} binary to ${CMAKE_CURRENT_SOURCE_DIR}/build/")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# copy fxcm cfg file to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/fxcm.cfg ${CMAKE_CURRENT_SOURCE_DIR}/build/)
	message(STATUS "POST_BUILD copy /specs/fxcm.cfg to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
	# copy fixfxcm10.xml to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/FIXFXCM10.xml ${CMAKE_CURRENT_SOURCE_DIR}/build/)
	message(STATUS "POST_BUILD copy /specs/FIXFXCM10.xml to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
endif()

# make public_html directory and copy everything to it
# set(CDIR ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html)
#file(MAKE_DIRECTORY ${CDIR} ${CDIR}/js ${CDIR}/css ${CDIR}/img )
#message(STATUS "POST_BUILD make directory ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html/")
	
#file(GLOB_RECURSE SRC_HTML src_html/*.html src_html/*.css src_html/*.js src_html/*.png)
#foreach(filename ${SRC_HTML})
#	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/src_html" "${CMAKE_CURRENT_SOURCE_DIR}/build/public_html" PUBLIC_HTML_PATH ${filename})
#	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${filename} ${PUBLIC_HTML_PATH}) 
#	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" PUBLIC_HTML_PATH_MSG ${filename})
#	message(STATUS "POST_BUILD copy ${PUBLIC_HTML_PATH_MSG} to /build/public_html")
#endforeach()

# install idefix in /usr/local/bin
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
