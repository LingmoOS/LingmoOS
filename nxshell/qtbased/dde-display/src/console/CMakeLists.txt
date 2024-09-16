
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_compile_options(-DQT_NO_KEYWORDS)

find_package(Qt5 REQUIRED COMPONENTS Core DBus Concurrent Gui)
find_package(KF5Screen REQUIRED)
find_package(KF5Wayland REQUIRED)

add_executable(dde-display-console main.cpp console.cpp dpmsclient.cpp console.h dpmsclient.h)

target_include_directories(dde-display-console PUBLIC
    ${Qt5DBus_INCLUDE_DIRS}
    ${KF5Screen_INCLUDE_DIRS}
)

target_link_libraries(dde-display-console
    Qt5::Core
    Qt5::DBus
    KF5::Screen
    KF5::WaylandClient
)

install(TARGETS dde-display-console DESTINATION bin)
