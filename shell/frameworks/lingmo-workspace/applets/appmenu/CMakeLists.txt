add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.appmenu\")

add_subdirectory(lib)
add_subdirectory(plugin)

lingmo_install_package(package org.kde.lingmo.appmenu)
