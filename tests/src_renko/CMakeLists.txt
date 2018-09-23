#
# renko BUILD
#

include_directories(/usr/local/include)
include_directories(../include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix ")

# add source files for your binary
add_executable(renko Strategy.h RenkoStrategy.h ../src/indicator/Tick.h ../src/indicator/Renko.h ../src/indicator/Renko.cpp ../src/indicator/IMovingAverage.h ../src/indicator/SimpleMovingAverage.h ../src/indicator/SimpleMovingAverage.cpp renko.cpp)

# quickfix linker
target_link_libraries(renko /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET renko POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:renko> ../)
# copy html to html output directory
add_custom_command(TARGET renko POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ../html)
add_custom_command(TARGET renko POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/html/chart.html ../html/)
add_custom_command(TARGET renko POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/html/anychart.min.js ../html/)