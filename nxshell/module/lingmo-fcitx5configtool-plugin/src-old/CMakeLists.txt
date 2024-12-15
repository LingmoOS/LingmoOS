include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
option(ENABLE_KCM "Enable KDE Config Module" On)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets X11Extras Concurrent)
find_package(DtkCore REQUIRED)
find_package(DtkWidget REQUIRED)

find_package(KF5ItemViews REQUIRED)

find_package(Fcitx5Qt5DBusAddons REQUIRED)
find_package(Fcitx5Qt5WidgetsAddons 5.0.2 REQUIRED)
find_package(Qt5Svg)
set_package_properties(Qt5Svg PROPERTIES
    DESCRIPTION "May be used for show icons"
    TYPE RUNTIME)

get_target_property(FCITX5_QT5_GUI_WRAPPER Fcitx5Qt5::gui-wrapper IMPORTED_LOCATION)

find_package(KF5WidgetsAddons REQUIRED)
kde_enable_exceptions()

configure_file(config.h.in config.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(layout)
