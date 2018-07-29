#
# GUI BUILD
#

include_directories(/usr/local/include)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -lncurses)

# add source files for your binary
add_executable(guitest gui.h gui.cpp main.cpp)

# copy binary to parent directory build/
add_custom_command(TARGET guitest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:guitest> ../)
