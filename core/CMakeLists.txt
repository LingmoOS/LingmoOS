cmake_minimum_required(VERSION 3.16)
project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

set(QT Core Gui Widgets Quick QuickControls2 DBus Xml LinguistTools)
find_package(Qt6 REQUIRED COMPONENTS ${QT})
# Find individual KF6 package configurations; OBS does not provide ECM's FindKF6.cmake module.
find_package(KF6CoreAddons REQUIRED)
find_package(KF6IdleTime REQUIRED)
find_package(KF6WindowSystem REQUIRED)

# Qt 6 removed QtX11Extras; X11-only code uses the local native-interface adapter.
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
# find_package(LingmoUI REQUIRED)
find_package(PkgConfig REQUIRED)

include(GNUInstallDirs)

add_subdirectory(polkit-agent)
add_subdirectory(screen-brightness)
add_subdirectory(session)
add_subdirectory(settings-daemon)
add_subdirectory(shutdown-ui)
add_subdirectory(xembed-sni-proxy)
add_subdirectory(powerman)
add_subdirectory(cpufreq)
add_subdirectory(cupdatecursor)
add_subdirectory(gmenuproxy)
add_subdirectory(notificationd)
add_subdirectory(sddm-helper)
add_subdirectory(clipboard)
add_subdirectory(chotkeys)

install(FILES lingmo DESTINATION /etc/ COMPONENT Runtime)
install(FILES Desktop/Lingmo DESTINATION /etc/Desktop/ COMPONENT Runtime)
install(FILES wallpaper-color-pick/lingmo-wallpaper-color-pick DESTINATION "/usr/bin/")
