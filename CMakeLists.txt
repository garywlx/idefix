# Require cmake version
cmake_minimum_required(VERSION 3.9)

# set c++ standard
set(CMAKE_CXX_STANDARD 11)

# set build type
#set(CMAKE_BUILD_TYPE Debug)

# Change this to your project name
project(idefix VERSION 0.1.0 LANGUAGES CXX)

# import src/CMakeLists.txt
add_subdirectory(src)

# import src_signals/CMakeLists.txt 
#add_subdirectory( src_signals )

# import src_pipnl/CMakeLists.txt
#add_subdirectory( src_pipnl )

# import src_mathtest/CMakeLists.txt
#add_subdirectory( src_mathtest )

# import src_timetest/CMakeLists.txt
#add_subdirectory( src_timetest )

# import src_gui/CMakeLists.txt
#add_subdirectory( src_gui )

# import src_tradelog/CMakeLists.txt
#add_subdirectory( src_tradelog )

# import src_playground/CMakeLists.txt
#add_subdirectory( src_playground )

# import src_renko/CMakeLists.txt
#add_subdirectory( src_renko )