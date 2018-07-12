#
# QUICKFIX BUILD
#

include_directories(/usr/local/include)

## main sources
#file(GLOB main_src ./*.h ./*.cpp)

# quickfix flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -fexceptions -finline-functions") #-lpthread -lxml2 -lz

# add source files for your binary
add_executable(${PROJECT_NAME} main.cpp FXCMFields.h FIXFactory.h FIXManager.h FIXManager.cpp times.h RequestId.h RequestId.cpp string_extensions.h MarketOrder.h MarketSnapshot.h Market.h TradeMath.h)

# quickfix linker
target_link_libraries(${PROJECT_NAME} /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/fxcm.cfg ../)
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/FIXFXCM10.xml ../)
