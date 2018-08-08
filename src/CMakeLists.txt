#
# QUICKFIX BUILD
#

include_directories(/usr/local/include)
include_directories(../include)

# quickfix flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -I../include/spdlog -fexceptions -finline-functions") #-lpthread -lxml2 -lz

set(SRC main.cpp Tick.h Candle.h Candle.cpp RiskManagement.h Math.h Pairs.h Account.h Account.cpp MarketDetail.h FXCMFields.h FIXFactory.h 
	FIXManager.h FIXManager.cpp times.h RequestId.h RequestId.cpp String.h MarketOrder.h MarketSnapshot.h Market.h
	Strategy.h RangeStrategy.h RangeStrategy.cpp)

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# quickfix linker
target_link_libraries(${PROJECT_NAME} /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/fxcm.cfg ../)
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/FIXFXCM10.xml ../)
