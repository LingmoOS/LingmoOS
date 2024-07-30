add_definitions(-DTRANSLATION_DOMAIN=\"plasma_applet_org.kde.plasma.comic\")

add_subdirectory(engine)

set(comic_SRCS
    comic.cpp
    comicmodel.cpp
    checknewstrips.cpp
    comicdata.cpp
    comicinfo.cpp
    comicsaver.cpp
    stripselector.cpp
    activecomicmodel.cpp
)

kcoreaddons_add_plugin(org.kde.plasma.comic SOURCES ${comic_SRCS} INSTALL_NAMESPACE "plasma/applets")

target_link_libraries(org.kde.plasma.comic
    Qt::Gui
    Qt::Widgets
    Plasma::Plasma
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications
    KF6::XmlGui
    plasma_engine_comic
)

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
plasma_install_package(package org.kde.plasma.comic)

