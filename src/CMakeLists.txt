##
# QUICKFIX BUILD
##
if(NOT PROJECT_NAME)
	set(PROJECT_NAME idefix)
endif(NOT PROJECT_NAME)

# quickfix flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/include/quickfix -I../include/spdlog -fexceptions -finline-functions") #-lpthread -lxml2 -lz

#set unix specific flags
if(NOT APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread")
endif(NOT APPLE)

set(SRC main.cpp 
	RiskManagement.h MathHelper.h Pairs.h MarketDetail.h FXCMFields.h FIXFactory.h times.h StringHelper.h 
	MarketOrder.h MarketSnapshot.h Market.h Operators.h Exceptions.h SignalType.h MarketSide.h
	Account.h Account.cpp 
	FIXManager.h FIXManager.cpp 
	RequestId.h RequestId.cpp 
	RenkoChart.h RenkoChart.cpp 
	AwesomeStrategy.h AwesomeStrategy.cpp
	CSVHandler.h CSVHandler.cpp
	Bar.h
	RenkoBrick.h 
	AbstractMovingAverage.h
	SimpleMovingAverage.h SimpleMovingAverage.cpp )

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# linker
if(APPLE)
	set(QUICKFIXLIB "/usr/local/lib/libquickfix.dylib")
elseif(NOT APPLE)
	set(QUICKFIXLIB "/usr/local/lib/libquickfix.so -lpthread")
endif(APPLE)

target_link_libraries(${PROJECT_NAME} ${QUICKFIXLIB})

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/fxcm.cfg ../)
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/FIXFXCM10.xml ../)

# create folder build/public_html
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory ../public_html)
# copy chart.html to parent directory build/public_html
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/html/chart.html ../public_html)
