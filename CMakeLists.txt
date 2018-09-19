# Require cmake version
cmake_minimum_required(VERSION 3.7)

# include directories
include_directories("include")

# set c++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# unix compiling
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
#add_compile_options("-Wall" "-Wextra" "-Werror" "-Wno-c++98-compat" "-I/usr/local/llvm-6.0.1/include/c++/v1")
#add_compile_options("-Wno-c++98-compat" "-I/usr/lib/llvm-6.0/include/c++/v1")
add_compile_options("-Wno-c++98-compat" "-std=c++11")

# set build type
set(CMAKE_BUILD_TYPE Debug)

# Change this to your project name
project(idefix VERSION 0.1.0 LANGUAGES CXX)

# quickfix flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -I../include/spdlog -fexceptions -stdlib=libc++") # -finline-functions") #-lpthread -lxml2 -lz
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

# linker
if(APPLE)
	set(QUICKFIXLIB "/usr/local/lib/libquickfix.dylib")
	target_link_libraries(${PROJECT_NAME} 
				${QUICKFIXLIB}
	)
elseif(NOT APPLE)
	find_package(Threads REQUIRED)
	set(QUICKFIXLIB "/usr/local/lib/libquickfix.so.16")
	target_link_libraries(${PROJECT_NAME} 
				${QUICKFIXLIB} 
				${CMAKE_THREAD_LIBS_INIT}
	)
endif(APPLE)

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/fxcm.cfg ../)
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/FIXFXCM10.xml ../)

# create folder build/public_html
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ../public_html)
# copy chart.html to parent directory build/public_html
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/html/chart.html ../public_html)
