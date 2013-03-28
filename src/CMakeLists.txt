# Enable c++11
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++11)
endif()

include_directories(${TWITTERFU_SOURCES_DIR}/libtwitcurl)
link_directories(${TWITTERFU_BINARY_DIR}/src)

add_executable(twitterfu ${twitterfuSrcs})
target_link_libraries(twitterfu libtwitcurl curl sqlite3)

install (TARGETS twitterfu DESTINATION bin)
