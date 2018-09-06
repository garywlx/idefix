#
# QUICKFIX BUILD
#

# quickfix flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -I../include/spdlog -fexceptions -finline-functions") #-lpthread -lxml2 -lz

set(SRC main.cpp 
	RiskManagement.h MathHelper.h Pairs.h MarketDetail.h FXCMFields.h FIXFactory.h times.h StringHelper.h 
	MarketOrder.h MarketSnapshot.h Market.h Operators.h Exceptions.h SignalType.h MarketSide.h
	Account.h Account.cpp 
	FIXManager.h FIXManager.cpp 
	RequestId.h RequestId.cpp 
	RenkoChart.h RenkoChart.cpp 
	AwesomeStrategy.h AwesomeStrategy.cpp
	Bar.h
	RenkoBrick.h 
	AbstractMovingAverage.h
	SimpleMovingAverage.h SimpleMovingAverage.cpp )

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# linker
target_link_libraries(${PROJECT_NAME} /usr/local/lib/libquickfix.dylib)

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/fxcm.cfg ../)
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/FIXFXCM10.xml ../)

# copy chart.html to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/html/chart.html ../)