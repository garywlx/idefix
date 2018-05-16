# Require cmake version
cmake_minimum_required(VERSION 3.9)
# Change this to your project name
project(idefix VERSION 0.1 LANGUAGES C CXX)
# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
#include(GNUInstallDirs)
# set c++ standard
set(CMAKE_CXX_STANDARD 11)
# import src/CMakeLists.txt
add_subdirectory(src)