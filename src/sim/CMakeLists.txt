###
### SIMULATION
###

# set lib name
set(SIM_TARGET simulator)

# set source files
set(SIM_SRC_FILES "main.cpp;sim.cpp;")

# add library
add_executable(${SIM_TARGET} ${SIM_SRC_FILES})

# link libraries
target_link_libraries(${SIM_TARGET} ${QUICKFIX_LIBRARY_PATH})

set(SIM_BUILD_DIRECTORY "${CMAKE_HOME_DIRECTORY}/build/sim")
file(MAKE_DIRECTORY ${SIM_BUILD_DIRECTORY})

# copy_if_different binary to build/sim/
add_custom_command(TARGET ${SIM_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${SIM_TARGET}> ${SIM_BUILD_DIRECTORY})
message(STATUS "POST_BUILD copy_if_different ${SIM_TARGET} to ${SIM_BUILD_DIRECTORY}")
# copy_if_different specs/sim.conf to build/sim/
add_custom_command(TARGET ${SIM_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/specs/sim.conf ${SIM_BUILD_DIRECTORY})
message(STATUS "POST_BUILD copy_if_different /specs/sim.conf to ${SIM_BUILD_DIRECTORY}")
# copy_if_different fixfxcm10.xml to build/
add_custom_command(TARGET ${SIM_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/specs/FIXFXCM10.xml ${SIM_BUILD_DIRECTORY})
message(STATUS "POST_BUILD copy_if_different /specs/FIXFXCM10.xml to ${SIM_BUILD_DIRECTORY}")
