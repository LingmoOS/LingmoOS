add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_applet_org.kde.lingmo.comic\")

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

kcoreaddons_add_plugin(org.kde.lingmo.comic SOURCES ${comic_SRCS} INSTALL_NAMESPACE "lingmo/applets")

target_link_libraries(org.kde.lingmo.comic
    Qt::Gui
    Qt::Widgets
    Lingmo::Lingmo
    KF6::I18n
    KF6::KIOCore
    KF6::KIOWidgets
    KF6::Notifications
    KF6::XmlGui
    lingmo_engine_comic
)

install(FILES comic.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
lingmo_install_package(package org.kde.lingmo.comic)

