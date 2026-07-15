project(platformthemeplugin)
include(GNUInstallDirs)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(PkgConfig REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets QuickControls2 DBus)
find_package(KF6WindowSystem REQUIRED)

# Qt 6.8 on Debian does not ship the private QtThemeSupport library. Keep
# the copied DBus global-menu backend for distributions that do, but disable
# it by default so applications retain their normal in-window menu bars.
option(BUILD_DBUS_GLOBAL_MENU "Build the private QtThemeSupport DBus global-menu backend" OFF)
if(BUILD_DBUS_GLOBAL_MENU)
    find_package(Qt6ThemeSupport REQUIRED)
    # qdbusmenubar uses Qt private DBus menu classes.
    remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)
endif()

pkg_check_modules(XCB_EWMH REQUIRED xcb xcb-ewmh x11)

set(SRCS
    main.cpp
    platformtheme.h
    platformtheme.cpp
    hintsettings.h
    hintsettings.cpp
    x11integration.h
    x11integration.cpp
)

if(BUILD_DBUS_GLOBAL_MENU)
    list(APPEND SRCS
        qdbusmenubar_p.h
        qdbusmenubar.cpp
    )
endif()

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

get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "Qt 6 qmake is not found.")
endif()

# Debian ships QPA headers in qt6-base-private-dev but does not export a Qt6::GuiPrivate CMake target.
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_HEADERS
    OUTPUT_VARIABLE QT_HEADERS_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_VERSION
    OUTPUT_VARIABLE QT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(QT_GUI_QPA_INCLUDE_DIR "${QT_HEADERS_DIR}/QtGui/${QT_VERSION}/QtGui")
if(NOT EXISTS "${QT_GUI_QPA_INCLUDE_DIR}/qpa/qplatformtheme.h")
    message(FATAL_ERROR "Qt 6 QPA headers are missing; install qt6-base-private-dev.")
endif()

add_library(lingmoplatformtheme MODULE ${SRCS})

target_compile_definitions(lingmoplatformtheme
    PRIVATE
        QT_NO_FOREACH
        LIB_FM_QT_SONAME="${LIB_FM_QT_SONAME}"
)

if(BUILD_DBUS_GLOBAL_MENU)
    target_compile_definitions(lingmoplatformtheme PRIVATE LINGMO_HAVE_DBUS_GLOBAL_MENU)
endif()


target_include_directories(lingmoplatformtheme PRIVATE "${QT_GUI_QPA_INCLUDE_DIR}")

target_link_libraries(lingmoplatformtheme PRIVATE
    Qt6::Gui
    Qt6::Widgets
    Qt6::QuickControls2
    Qt6::Core
    Qt6::DBus
    KF6::WindowSystem
    ${XCB_LIBRARIES}
)

if(BUILD_DBUS_GLOBAL_MENU)
    target_link_libraries(lingmoplatformtheme PRIVATE Qt6ThemeSupport::Qt6ThemeSupport)
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
