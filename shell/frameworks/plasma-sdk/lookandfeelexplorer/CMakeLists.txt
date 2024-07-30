add_definitions(-DTRANSLATION_DOMAIN=\"org.kde.plasma.lookandfeelexplorer\")

add_subdirectory(src)

kpackage_install_package(package org.kde.plasma.lookandfeelexplorer genericqml)

