#
# renko BUILD
#

include_directories(/usr/local/include)
include_directories(../include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix ")

# Qt
# Set this to your Qt installation
#set(CMAKE_PREFIX_PATH ~/sourcecode/sdk/QtLatest/5.9.2/clang_64)
#set(RESOURCE_FOLDER res)
#set(RESOURCE_FILES ${RESOURCE_FOLDER}/resources.qrc)
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
#set(CMAKE_AUTORCC ON)

#find_package(Qt5Core REQUIRED)
#find_package(Qt5Widgets REQUIRED)
#find_package(Qt5Charts REQUIRED)
#find_package(Qt5Core REQUIRED)

# add source files for your binary
add_executable(renkonvert ../src/indicator/IMovingAverage.h ../src/indicator/SimpleMovingAverage.h ../src/indicator/SimpleMovingAverage.cpp renkonvert.cpp)
add_executable(renkotest ../src/indicator/IMovingAverage.h ../src/indicator/SimpleMovingAverage.h ../src/indicator/SimpleMovingAverage.cpp renkotest.cpp)

# quickfix linker
target_link_libraries(renkonvert /usr/local/lib/libquickfix.dylib)
target_link_libraries(renkotest /usr/local/lib/libquickfix.dylib) # Qt5::Core Qt5::Widgets Qt5::Charts

# copy binary to parent directory build/
add_custom_command(TARGET renkonvert POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:renkonvert> ../)
add_custom_command(TARGET renkotest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:renkotest> ../)
