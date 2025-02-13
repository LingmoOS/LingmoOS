project(platformthemeplugin)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5QuickControls2 REQUIRED)
find_package(Qt5DBus REQUIRED)
find_package(Qt5X11Extras REQUIRED)
find_package(Qt5Gui CONFIG REQUIRED Private)
find_package(Qt5XdgIconLoader REQUIRED)
find_package(dbusmenu-qt5 REQUIRED)
find_package(KF5WindowSystem REQUIRED)

# dependencies for QPA plugin
find_package(Qt5ThemeSupport REQUIRED)
set(QT5PLATFORMSUPPORT_LIBS Qt5ThemeSupport::Qt5ThemeSupport)

# qdbusmenubar uses them
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

pkg_check_modules(XCB_EWMH REQUIRED xcb xcb-ewmh x11)

set (SRCS
    main.cpp
    platformtheme.h
    platformtheme.cpp
    hintsettings.h
    hintsettings.cpp
    systemtrayicon.h
    systemtrayicon.cpp
    qdbusmenubar_p.h
    qdbusmenubar.cpp
    x11integration.h
    x11integration.cpp
    statusnotifier/dbustypes.h
    statusnotifier/dbustypes.cpp
    statusnotifier/statusnotifieritem.h
    statusnotifier/statusnotifieritem.cpp
)

qt5_add_dbus_interface(SRCS org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)

qt5_add_dbus_adaptor(SRCS
    statusnotifier/org.kde.StatusNotifierItem.xml
    statusnotifier/statusnotifieritem.h
    StatusNotifierItem
)

add_library(lingmoplatformtheme MODULE ${SRCS})

target_compile_definitions(lingmoplatformtheme
    PRIVATE
        "QT_NO_FOREACH"
        "LIB_FM_QT_SONAME=\"${LIB_FM_QT_SONAME}\""
)

target_link_libraries(lingmoplatformtheme PRIVATE
    Qt5::GuiPrivate
    Qt5::X11Extras
    Qt5::Widgets
    Qt5::QuickControls2
    Qt5::Core
    Qt5::DBus
    dbusmenu-qt5

    Qt5XdgIconLoader

    KF5::WindowSystem
    ${XCB_LIBRARIES}
    ${QT5PLATFORMSUPPORT_LIBS}
)

get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif()

# execute the command "qmake -query QT_INSTALL_PLUGINS" to get the path of plugins dir.
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_PLUGINS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(QT_PLUGINS_DIR)
    message(STATUS "Qt5 plugin directory:" "${QT_PLUGINS_DIR}")
else()
    message(FATAL_ERROR "Qt5 plugin directory cannot be detected.")
endif()

install(TARGETS lingmoplatformtheme LIBRARY DESTINATION "${QT_PLUGINS_DIR}/platformthemes")
