#
# pipnl BUILD
#

include_directories(/usr/local/include)

# add source files for your binary
add_executable(pipnl main.cpp)

# copy binary to parent directory build/
add_custom_command(TARGET pipnl POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pipnl> ../)
