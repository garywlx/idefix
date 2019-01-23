# get all source files
file(GLOB SRC_FILES *.cpp core/*.cpp db/*.cpp)
# get all header files 
file(GLOB SRC_HEADER_FILES *.h core/*.h db/*.h)
# add executable
add_executable(${PROJECT_NAME} ${SRC_FILES} ${SRC_HEADER_FILES})
# add target link libraries
target_link_libraries(${PROJECT_NAME}
	${QUICKFIX_LIBRARY_PATH}
	${Boost_LIBRARIES}
	dl pthread crypto
)
# add target include directories
target_include_directories(${PROJECT_NAME} PRIVATE core)
target_include_directories(${PROJECT_NAME} PRIVATE db)
target_include_directories(${PROJECT_NAME} PRIVATE webinterface)

# set version MAJOR.MINOR.PATCH
set(PROJECT_VERSION 0.2.8)

# broadcast version definition to source
target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_PROJECT_VERSION="${PROJECT_VERSION}")

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_HOME_DIRECTORY}/build/)
message(STATUS "POST_BUILD copy ${PROJECT_NAME} binary to ${CMAKE_HOME_DIRECTORY}/build/")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# copy fxcm cfg file to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/specs/fxcm.conf ${CMAKE_HOME_DIRECTORY}/build/)
	message(STATUS "POST_BUILD copy /specs/fxcm.conf to ${CMAKE_HOME_DIRECTORY}/build/")
	# copy fixfxcm10.xml to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/specs/FIXFXCM10.xml ${CMAKE_HOME_DIRECTORY}/build/)
	message(STATUS "POST_BUILD copy /specs/FIXFXCM10.xml to ${CMAKE_HOME_DIRECTORY}/build/")
	# copy idefix.conf to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/specs/idefix.conf ${CMAKE_HOME_DIRECTORY}/build/)
	message(STATUS "POST_BUILD copy /specs/idefix.conf to ${CMAKE_HOME_DIRECTORY}/build/")
endif()

# install idefix in /usr/local/bin
install(TARGETS ${PROJECT_NAME} DESTINATION bin)

# add adapter modules
add_subdirectory(adapter)