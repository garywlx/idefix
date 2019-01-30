# set lib name
set(FXCM_LIB_TARGET fxcm)

# set source files
set(FXCM_SRC_FILES "fxcm.cpp;${CMAKE_HOME_DIRECTORY}/src/core/exchangetypes.h;${CMAKE_HOME_DIRECTORY}/src/core/instrument.cpp;${CMAKE_HOME_DIRECTORY}/src/core/order.cpp;")

# add library
add_library(${FXCM_LIB_TARGET} MODULE ${FXCM_SRC_FILES})
# link libraries
target_link_libraries(${FXCM_LIB_TARGET} ${QUICKFIX_LIBRARY_PATH})

# copy libfxcm to build/adapter/
add_custom_command(TARGET ${FXCM_LIB_TARGET} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${FXCM_LIB_TARGET}> ${ADAPTER_BUILD_DIRECTORY})
message(STATUS "POST_BUILD copy lib${FXCM_LIB_TARGET} to ${ADAPTER_BUILD_DIRECTORY}")
