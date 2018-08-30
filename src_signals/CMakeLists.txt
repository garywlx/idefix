#
# signals BUILD
#

include_directories(/usr/local/include)
include_directories(../include)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix ")

# add source files for your binary
add_executable(signals main.cpp)

# quickfix linker
#target_link_libraries(signals /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET signals POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:signals> ../)
