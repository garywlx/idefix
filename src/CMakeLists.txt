file(GLOB SRC_FILES *.cpp core/*.cpp db/*.cpp webinterface/*.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})

target_link_libraries(${PROJECT_NAME}
	${QUICKFIX_LIBRARY_PATH}
	${Boost_LIBRARIES}
	dl pthread crypto
)

# set version MAJOR.MINOR.PATCH
set(PROJECT_VERSION 0.2.8)

# broadcast version definition to source
target_compile_definitions(${PROJECT_NAME} PUBLIC CMAKE_PROJECT_VERSION="${PROJECT_VERSION}")

# copy binary to parent directory build/
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${CMAKE_HOME_DIRECTORY}/build/)
message(STATUS "POST_BUILD copy ${PROJECT_NAME} binary to ${CMAKE_HOME_DIRECTORY}/build/")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# copy fxcm cfg file to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/specs/fxcm.cfg ${CMAKE_HOME_DIRECTORY}/build/)
	message(STATUS "POST_BUILD copy /specs/fxcm.cfg to ${CMAKE_HOME_DIRECTORY}/build/")
	# copy fixfxcm10.xml to build/
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_HOME_DIRECTORY}/specs/FIXFXCM10.xml ${CMAKE_HOME_DIRECTORY}/build/)
	message(STATUS "POST_BUILD copy /specs/FIXFXCM10.xml to ${CMAKE_HOME_DIRECTORY}/build/")
endif()

# install idefix in /usr/local/bin
install(TARGETS ${PROJECT_NAME} DESTINATION bin)