add_definitions(-DTRANSLATION_DOMAIN=\"org.kde.lingmo.themeexplorer\")

add_subdirectory(src)

kpackage_install_package(package org.kde.lingmo.themeexplorer genericqml)

