cmake_minimum_required (VERSION 2.6) 
project (TWITTERFU) 

# Recurse into the "Hello" and "Demo" subdirectories. This does not actually 
# cause another cmake executable to run. The same process will walk through 
# the project's entire directory structure. 
#add_subdirectory (libtwitcurl)
FIND_PACKAGE(PkgConfig)
pkg_check_modules(boost sqlite3 libcurl)
include_directories(${PKG_INCLUDE_DIRS})

#gtkmm
pkg_check_modules(GTKMM gtkmm-3.0)
include_directories(${GTKMM_INCLUDE_DIRS})
link_directories(${GTKMM_LIBRARY_DIRS})


set(libtwitcurlSrcs libtwitcurl/base64.cpp libtwitcurl/HMAC_SHA1.cpp libtwitcurl/oauthlib.cpp libtwitcurl/SHA1.cpp libtwitcurl/urlencode.cpp libtwitcurl/twitcurl.cpp)
add_library (libtwitcurl STATIC ${libtwitcurlSrcs})
set(twitterfuSrcs User.cpp Database.cpp Proxy.cpp Filters.cpp twitterfu.cpp sqlite3pp.cpp GtkTwitterfu.cpp)
add_subdirectory (src)

