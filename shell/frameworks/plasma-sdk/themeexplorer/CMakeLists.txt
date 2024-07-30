add_definitions(-DTRANSLATION_DOMAIN=\"org.kde.plasma.themeexplorer\")

add_subdirectory(src)

kpackage_install_package(package org.kde.plasma.themeexplorer genericqml)

