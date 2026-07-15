project(platformthemeplugin)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui GuiPrivate Widgets QuickControls2 DBus)
find_package(Qt6XdgIconLoader REQUIRED)
find_package(KF6WindowSystem REQUIRED)
find_package(Qt6ThemeSupport REQUIRED)

# qdbusmenubar uses Qt private DBus menu classes.
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

pkg_check_modules(XCB_EWMH REQUIRED xcb xcb-ewmh x11)

set(SRCS
    main.cpp
    platformtheme.h
    platformtheme.cpp
    hintsettings.h
    hintsettings.cpp
    qdbusmenubar_p.h
    qdbusmenubar.cpp
    x11integration.h
    x11integration.cpp
)

list(APPEND SRCS
    systemtrayicon.h
    systemtrayicon.cpp
    statusnotifier/dbusmenuexporter.h
    statusnotifier/dbusmenuexporter.cpp
    statusnotifier/dbustypes.h
    statusnotifier/dbustypes.cpp
    statusnotifier/statusnotifieritem.h
    statusnotifier/statusnotifieritem.cpp
)

qt_add_dbus_interface(SRCS org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)
qt_add_dbus_adaptor(SRCS
    statusnotifier/org.kde.StatusNotifierItem.xml
    statusnotifier/statusnotifieritem.h
    StatusNotifierItem
)

add_library(lingmoplatformtheme MODULE ${SRCS})

target_compile_definitions(lingmoplatformtheme
    PRIVATE
        QT_NO_FOREACH
        LIB_FM_QT_SONAME="${LIB_FM_QT_SONAME}"
)


target_link_libraries(lingmoplatformtheme PRIVATE
    Qt6::GuiPrivate
    Qt6::Widgets
    Qt6::QuickControls2
    Qt6::Core
    Qt6::DBus
    Qt6XdgIconLoader
    KF6::WindowSystem
    ${XCB_LIBRARIES}
    Qt6ThemeSupport::Qt6ThemeSupport
)


get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "Qt 6 qmake is not found.")
endif()

# Execute "qmake -query QT_INSTALL_PLUGINS" to use the Qt installation's
# platform-theme plugin location.
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
    OUTPUT_VARIABLE QT_PLUGINS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(QT_PLUGINS_DIR)
    message(STATUS "Qt 6 plugin directory: ${QT_PLUGINS_DIR}")
else()
    message(FATAL_ERROR "Qt 6 plugin directory cannot be detected.")
endif()

install(TARGETS lingmoplatformtheme LIBRARY DESTINATION "${QT_PLUGINS_DIR}/platformthemes")
