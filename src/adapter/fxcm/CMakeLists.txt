file(GLOB_RECURSE SRC_FILES *.cpp)
add_library(fxcm MODULE ${SRC_FILES})
target_link_libraries(fxcm ${QUICKFIX_LIBRARY_PATH})