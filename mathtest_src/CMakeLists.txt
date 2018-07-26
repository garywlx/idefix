#
# mathtest BUILD
#

include_directories(/usr/local/include)

# add source files for your binary
add_executable(mathtest main.cpp ../src/Math.h)

# copy binary to parent directory build/
add_custom_command(TARGET mathtest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:mathtest> ../)
