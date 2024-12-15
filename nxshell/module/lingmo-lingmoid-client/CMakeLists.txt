cmake_minimum_required(VERSION 3.12)

project(lingmo-lingmoid-client
  VERSION "7.0.3"
  DESCRIPTION "lingmo-lingmoid-client"
  HOMEPAGE_URL "https://github.com/lingmoos/lingmo-lingmoid-client"
  LANGUAGES CXX C
)
 
option(ENABLE_QT6 "Build Qt6 statically or dynamically" OFF)
set(ENABLE_QT6 OFF CACHE BOOL "enable Qt6 option")
if (ENABLE_QT6)
    set(QT_VERSION_MAJOR "6")
    set(DTK_VERSION_MAJOR "6")
else()
    set(QT_VERSION_MAJOR "5")
endif()
message("Qt Version:" ${QT_VERSION_MAJOR})
add_subdirectory(src)

add_subdirectory(examples/unionid-app)
