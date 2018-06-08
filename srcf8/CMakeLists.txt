#
# FIX8 BUILD
#

include_directories(/usr/local/include)

# add all source files to a variable
## adapter sources
file(GLOB adapter_src adapter/*.h adapter/*.cpp adapter/fix8/idefix8*.hpp adapter/fix8/idefix8*.cpp)
## main sources
file(GLOB main_src *.h *.cpp)
set(project_source ${main_src} ${adapter_src})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -I../libs/fix8-1.4.0/include/fix8")

# add source files for your binary
add_executable(${PROJECT_NAME} ${project_source})

# quickfix linker
target_link_libraries(${PROJECT_NAME} /Users/arnegockeln/sourcecode/cpp/idefix/libs/poco-1.9.0-all/lib/Darwin/x86_64/libPocoNet.dylib /usr/local/lib/libfix8.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)