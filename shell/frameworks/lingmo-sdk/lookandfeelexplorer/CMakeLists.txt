add_definitions(-DTRANSLATION_DOMAIN=\"org.kde.lingmo.lookandfeelexplorer\")

add_subdirectory(src)

kpackage_install_package(package org.kde.lingmo.lookandfeelexplorer genericqml)

