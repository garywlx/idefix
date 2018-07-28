#
# timetest BUILD
#

include_directories(/usr/local/include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -fexceptions -finline-functions") #-lpthread -lxml2 -lz

set(SUBPROJECT_NAME timetest)

# add source files for your binary
add_executable(${SUBPROJECT_NAME} main.cpp)

# quickfix linker
target_link_libraries(${SUBPROJECT_NAME} /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET ${SUBPROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${SUBPROJECT_NAME}> ../)
