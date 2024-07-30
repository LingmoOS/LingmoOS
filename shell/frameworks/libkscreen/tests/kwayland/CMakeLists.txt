# FIXME KWaylandServer is not a thing in KF6
if (NOT TARGET KF6::WaylandServer)
    message(WARNING "Skipping KF6::WaylandServer based tests!")
    return()
endif()

add_definitions(-DTEST_DATA="${CMAKE_SOURCE_DIR}/autotests/configs/")

add_executable(waylandtestserver main.cpp waylandtestserver.cpp waylandconfigreader.cpp)
target_link_libraries(waylandtestserver KF6::Screen KF6::WaylandServer)

