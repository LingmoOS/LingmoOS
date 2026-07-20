cmake_minimum_required(VERSION 3.16)
project(core)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_compile_options(-Wall)
# add_compile_definitions(QT_DISABLE_DEPRECATED_BEFORE=0x050F00)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

set(QT Core Gui Widgets Quick QuickControls2 DBus Xml LinguistTools)
find_package(Qt6 REQUIRED ${QT})

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
