# The session's touchpad scrolling for Qt applications: the physics and hold
# gesture, and the widget and Qt Quick views the platform theme drives.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt6 REQUIRED COMPONENTS Gui Quick Widgets)
find_package(Qt6Gui CONFIG QUIET COMPONENTS Private)
if(NOT TARGET Qt6::GuiPrivate)
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
endif()
find_package(Qt6Quick CONFIG QUIET COMPONENTS Private)
if(NOT TARGET Qt6::QuickPrivate)
    find_package(Qt6 REQUIRED COMPONENTS QuickPrivate)
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(WaylandClient REQUIRED IMPORTED_TARGET wayland-client)
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
find_program(WAYLAND_SCANNER_EXECUTABLE NAMES wayland-scanner REQUIRED)

include(GNUInstallDirs)

# Standard protocols are generated privately, so their symbols cannot clash
# with the copies inside Qt.
set(protocol_dir ${CMAKE_CURRENT_BINARY_DIR}/protocols)
file(MAKE_DIRECTORY ${protocol_dir})
set(pointer_gestures_xml
    ${WAYLAND_PROTOCOLS_DATADIR}/unstable/pointer-gestures/pointer-gestures-unstable-v1.xml)
set(pointer_gestures_header ${protocol_dir}/pointer-gestures-unstable-v1-client-protocol.h)
set(pointer_gestures_code ${protocol_dir}/pointer-gestures-unstable-v1-protocol.c)
add_custom_command(OUTPUT ${pointer_gestures_header}
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} client-header ${pointer_gestures_xml} ${pointer_gestures_header}
    DEPENDS ${pointer_gestures_xml} VERBATIM
)
add_custom_command(OUTPUT ${pointer_gestures_code}
    COMMAND ${WAYLAND_SCANNER_EXECUTABLE} private-code ${pointer_gestures_xml} ${pointer_gestures_code}
    DEPENDS ${pointer_gestures_xml} VERBATIM
)
set_source_files_properties(${pointer_gestures_header} ${pointer_gestures_code} PROPERTIES SKIP_AUTOGEN ON)

# The physics and the hold gesture, for any kind of view.
add_library(lingmo-qt-scrolling SHARED
    scroller.cpp
    scroller.h
    scrollhold.cpp
    scrollhold.h
    ${pointer_gestures_header}
    ${pointer_gestures_code}
)
add_library(Lingmo::Scrolling ALIAS lingmo-qt-scrolling)
target_include_directories(lingmo-qt-scrolling
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    PRIVATE
        ${protocol_dir}
)
target_link_libraries(lingmo-qt-scrolling
    PUBLIC
        Qt6::Gui
    PRIVATE
        Qt6::GuiPrivate
        PkgConfig::WaylandClient
)

# Qt Quick views. A library of its own so widget applications never load Qt Quick.
add_library(lingmo-qt-scrolling-quick SHARED
    flickablescroller.cpp
    flickablescroller.h
    quickscrolling.cpp
    quickscrolling.h
)
add_library(Lingmo::ScrollingQuick ALIAS lingmo-qt-scrolling-quick)
target_link_libraries(lingmo-qt-scrolling-quick
    PUBLIC
        Lingmo::Scrolling
        Qt6::Quick
    PRIVATE
        Qt6::QuickPrivate
)

add_library(lingmo-qt-scrolling-widgets SHARED
    scrollareascroller.cpp
    scrollareascroller.h
    widgetscrolling.cpp
    widgetscrolling.h
)
add_library(Lingmo::ScrollingWidgets ALIAS lingmo-qt-scrolling-widgets)
target_link_libraries(lingmo-qt-scrolling-widgets
    PUBLIC
        Lingmo::Scrolling
        Qt6::Widgets
)

install(TARGETS
    lingmo-qt-scrolling
    lingmo-qt-scrolling-quick
    lingmo-qt-scrolling-widgets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
