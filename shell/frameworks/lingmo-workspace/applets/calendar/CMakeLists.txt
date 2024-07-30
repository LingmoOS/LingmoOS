kcoreaddons_add_plugin(org.kde.lingmo.calendar SOURCES calendarapplet.cpp calendarapplet.h INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.calendar
                      Lingmo::Lingmo)

lingmo_install_package(package org.kde.lingmo.calendar)
