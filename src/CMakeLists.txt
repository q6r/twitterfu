# Enable c++11
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++11)
endif()

link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

include_directories(${TWITTERFU_SOURCES_DIR}/libtwitcurl)
link_directories(${TWITTERFU_BINARY_DIR}/src)


add_executable(twitterfu ${twitterfuSrcs})
target_link_libraries(twitterfu libtwitcurl curl sqlite3 ${GTKMM_LIBRARIES})

install (TARGETS twitterfu DESTINATION bin)
