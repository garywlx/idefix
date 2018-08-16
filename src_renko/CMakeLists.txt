#
# renko BUILD
#

include_directories(/usr/local/include)
include_directories(../include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix ")

# add source files for your binary
add_executable(renkonvert renkonvert.cpp)
add_executable(renkotest renkotest.cpp)

# quickfix linker
target_link_libraries(renkonvert /usr/local/lib/libquickfix.dylib)
target_link_libraries(renkotest /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET renkonvert POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:renkonvert> ../)
add_custom_command(TARGET renkotest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:renkotest> ../)

