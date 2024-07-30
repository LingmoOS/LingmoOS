set(QT_MAJOR_VERSION 6)

include(KDEInstallDirs6)
include(KDECMakeSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Widgets DBus QuickControls2 WaylandClient)
find_package(Qt6Gui ${QT_MIN_VERSION} CONFIG REQUIRED Private)

# https://bugreports.qt.io/browse/QTBUG-114706
add_library(Qt::Core ALIAS Qt6::Core)
add_library(Qt::Network ALIAS Qt6::Network)
add_library(Qt::OpenGL ALIAS Qt6::OpenGL)
add_library(Qt::Gui ALIAS Qt6::Gui)
if(Qt6_VERSION_MINOR GREATER 6)
    add_library(Qt::PlatformModuleInternal ALIAS Qt6::PlatformModuleInternal)
endif()

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
                Config ConfigWidgets I18n IconThemes KIO Notifications
                WindowSystem GuiAddons XmlGui
                StatusNotifierItem
)

find_package(KF6ColorScheme ${KF6_MIN_VERSION} REQUIRED CONFIG)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)

find_package(KF6I18n CONFIG REQUIRED)
ki18n_install(../po)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
    add_subdirectory(tests)
endif()
