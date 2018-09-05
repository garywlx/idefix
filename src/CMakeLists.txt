#
# QUICKFIX BUILD
#

# Qt
# Set this to your Qt installation
set(CMAKE_PREFIX_PATH ~/sourcecode/sdk/QtLatest/5.9.2/clang_64)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5PrintSupport REQUIRED)

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
	QPlot.h QPlot.cpp
	qcustomplot.h qcustomplot.cpp
	Bar.h
	RenkoBrick.h 
	AbstractMovingAverage.h
	SimpleMovingAverage.h SimpleMovingAverage.cpp )

# add source files for your binary
add_executable(${PROJECT_NAME} ${SRC})

# linker
target_link_libraries(${PROJECT_NAME} /usr/local/lib/libquickfix.dylib Qt5::Core Qt5::Widgets Qt5::PrintSupport) 


# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ../)
# copy fxcm cfg file to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/fxcm.cfg ../)
# copy fixfxcm10.xml to build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ../../specs/FIXFXCM10.xml ../)
