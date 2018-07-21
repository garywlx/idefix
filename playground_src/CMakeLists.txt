#
# playground BUILD
#

include_directories(/usr/local/include)

# add source files for your binary
add_executable(playground main.cpp)

# copy binary to parent directory build/
add_custom_command(TARGET playground POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:playground> ../)
