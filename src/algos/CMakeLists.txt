# tick70 algo target
set(TICK70_LIB_TARGET "tick70")

# set source files
set(TICK70_SRC_FILES "tick70.cpp;${CMAKE_HOME_DIRECTORY}/src/core/datacontext.cpp;${CMAKE_HOME_DIRECTORY}/src/core/instrument.cpp;${CMAKE_HOME_DIRECTORY}/src/core/order.cpp;${CMAKE_HOME_DIRECTORY}/src/core/account.cpp;")

# add library
add_library(${TICK70_LIB_TARGET} MODULE ${TICK70_SRC_FILES})

# link libraries
target_link_libraries(${TICK70_LIB_TARGET} ${Boost_LIBRARIES})

# copy libfxcm to build/adapter/
add_custom_command(TARGET ${TICK70_LIB_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${TICK70_LIB_TARGET}> ${ALGO_BUILD_DIRECTORY})
message(STATUS "POST_BUILD copy_if_different lib${TICK70_LIB_TARGET} to ${ALGO_BUILD_DIRECTORY}")
