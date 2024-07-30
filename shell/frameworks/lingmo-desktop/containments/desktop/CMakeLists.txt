add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.desktopcontainment\")
add_subdirectory(plugins)
lingmo_install_package(package org.kde.desktopcontainment plasmoids containment)
