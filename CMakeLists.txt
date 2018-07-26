# Require cmake version
cmake_minimum_required(VERSION 3.9)

# set c++ standard
set(CMAKE_CXX_STANDARD 11)

# set build type
#set(CMAKE_BUILD_TYPE Debug)

# Change this to your project name
project(idefix VERSION 0.1 LANGUAGES CXX)

# import src/CMakeLists.txt
add_subdirectory(src)

# import pipnl_src/CMakeLists.txt
add_subdirectory(pipnl_src)

# import mathtest_src/CMakeLists.txt
add_subdirectory(mathtest_src)