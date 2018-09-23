# Require cmake version
cmake_minimum_required(VERSION 3.7)

# Change this to your project name
project(idefix VERSION 0.1.0 LANGUAGES CXX)

# set compiler
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)

# llvm project
#find_package(LLVM REQUIRED CONFIG)
#message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
#message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# set directories of custom includes
set(inc_QUICKFIX "/usr/local/include/quickfix")
set(inc_PROJECT_DIR "include")
set(inc_SPDLOG "include/spdlog")

# include directories
include_directories(src ${inc_QUICKFIX} ${inc_PROJECT_DIR})
#include_directories(${LLVM_INCLUDE_DIRS})
#add_definitions(${LLVM_DEFINITIONS})
#message(STATUS "Including llvm dirs: ${LLVM_INCLUDE_DIRS}")
#message(STATUS "Adding llvm definitions: ${LLVM_DEFINITIONS}")

# set c++ standard
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_compile_options("-Wno-c++98-compat") # "-stdlib=libc++")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${inc_QUICKFIX} -I${inc_NOD} -I${inc_SPDLOG} -v -Wno-c++98-compat -stdlib=libc++ -std=c++11 -fexceptions")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${inc_PROJECT_DIR} -I${inc_QUICKFIX} -fexceptions")

# set build type
set(CMAKE_BUILD_TYPE Debug)

# quickfix flags
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -I../include/spdlog -fexceptions -stdlib=libc++") # -finline-functions") #-lpthread -lxml2 -lz
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")

set(SRC src/main.cpp 
	src/RiskManagement.h src/MathHelper.h src/Pairs.h src/MarketDetail.h src/FXCMFields.h src/FIXFactory.h src/times.h src/StringHelper.h 
	src/MarketOrder.h src/MarketSnapshot.h src/Market.h src/Operators.h src/Exceptions.h src/SignalType.h src/MarketSide.h
	src/Account.h src/Account.cpp 
	src/FIXManager.h src/FIXManager.cpp 
	src/RequestId.h src/RequestId.cpp 
	src/RenkoChart.h src/RenkoChart.cpp 
	src/AwesomeStrategy.h src/AwesomeStrategy.cpp
	src/CSVHandler.h src/CSVHandler.cpp
	src/Bar.h
	src/RenkoBrick.h 
	src/AbstractMovingAverage.h
	src/SimpleMovingAverage.h src/SimpleMovingAverage.cpp )

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# Find the libraries that correspond to the LLVM components
# that we wish to use
#llvm_map_components_to_libnames(llvm_libs support core bitreader linker)

# linker
if(APPLE)
	set(LINK_LIBS /usr/local/lib/libquickfix.dylib)
elseif(NOT APPLE)
	find_package(Threads REQUIRED)
	set(LINK_LIBS /usr/local/lib/libquickfix.so ${CMAKE_THREAD_LIBS_INIT})
endif()

target_link_libraries(${PROJECT_NAME} ${LINK_LIBS}) #${llvm_libs}

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(STATUS "Copied binary ${PROJECT_NAME} to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/fxcm.cfg ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(STATUS "Copied ${CMAKE_CURRENT_SOURCE_DIR}/specs/fxcm.cfg to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/specs/FIXFXCM10.xml ${CMAKE_CURRENT_SOURCE_DIR}/build/)
message(STATUS "Copied ${CMAKE_CURRENT_SOURCE_DIR}/specs/FIXFXCM10.xml to ${CMAKE_CURRENT_SOURCE_DIR}/build/")
# create folder build/public_html
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html)
message(STATUS "Made directory ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html/")
# copy chart.html to parent directory build/public_html
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/src/html/chart.html ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html)
message(STATUS "Copied ${CMAKE_CURRENT_SOURCE_DIR}/src/html/chart.html to ${CMAKE_CURRENT_SOURCE_DIR}/build/public_html")

# install idefix in /usr/local/bin
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
